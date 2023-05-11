#include"SpectrumVisual.h"



namespace dsignal
{
	SpectrumVisual::~SpectrumVisual() {
		FreeDatas();
	}

	void SpectrumVisual::FreeDatas()
	{


		if (freq_of_bars != nullptr) {
			free(freq_of_bars);
			freq_of_bars = nullptr;
		}

		if (amp_of_bars != nullptr) {
			free(amp_of_bars);
			amp_of_bars = nullptr;
		}

		if (tmp_amp_of_bars != nullptr) {
			free(tmp_amp_of_bars);
			tmp_amp_of_bars = nullptr;
		}

		//
		if (input_buffer != nullptr) {
			free(input_buffer);
			input_buffer = nullptr;
		}

		//
		if (bass_mul != nullptr) free(bass_mul);
		if (mid_mul != nullptr) free(mid_mul);
		if (treble_mul != nullptr) free(treble_mul);
		bass_mul = mid_mul = treble_mul = nullptr;


		//
		FreeFFTInfo(bass_l);
		FreeFFTInfo(mid_l);
		FreeFFTInfo(treble_l);
		FreeFFTInfo(bass_r);
		FreeFFTInfo(mid_r);
		FreeFFTInfo(treble_r);
	}

	void SpectrumVisual::FreeFFTInfo(FFTInfo& fftInfo)
	{
		delete[]  fftInfo.in;
		delete[]  fftInfo.out;
		kiss_fftr_free(fftInfo.state);

		fftInfo.in = nullptr;
		fftInfo.out = nullptr;
		fftInfo.state = nullptr;
	}

	void SpectrumVisual::Init(
		int number_of_bars, int start_freq, int end_freq, unsigned int rate,
		int channels, int low_cut_off, int high_cut_off, int skip_frame)
	{

		//
		if (number_of_bars <= 0) number_of_bars = this->number_of_bars;
		if (rate <= 0) rate = this->rate;
		//
		if (start_freq < 0) start_freq = 0;
		if (end_freq > rate / 2) end_freq = rate / 2;
		if (start_freq > end_freq) {
			int tmp = start_freq;
			start_freq = end_freq;
			end_freq = tmp;
		}

		if (channels <= 0) channels = this->channels;

		if (low_cut_off <= 0) low_cut_off = 0;
		if (high_cut_off > rate / 2) high_cut_off = high_cut_off / 2;
		if (low_cut_off > high_cut_off) {
			int tmp = low_cut_off;
			low_cut_off = high_cut_off;
			high_cut_off = tmp;
		}

		this->skip_frame = skip_frame;
		curt_frame = 0;

		//
		if (this->number_of_bars == number_of_bars &&
			this->rate == rate && this->channels == channels)
		{
			if (this->start_freq != start_freq || this->end_freq != end_freq) {
				this->start_freq = start_freq;
				this->end_freq = end_freq;
				CreateBarFreqs();
			}

			this->start_freq = start_freq;
			this->end_freq = end_freq;
			this->low_cut_off = low_cut_off;
			this->high_cut_off = high_cut_off;

			return;
		}

		//
		FreeDatas();

		this->number_of_bars = number_of_bars;
		this->start_freq = start_freq;
		this->end_freq = end_freq;
		this->channels = channels;
		this->rate = rate;
		this->low_cut_off = low_cut_off;
		this->high_cut_off = high_cut_off;


		amp_of_bars = (double*)calloc(number_of_bars * 2, sizeof(double));
		tmp_amp_of_bars = (double*)calloc(number_of_bars * 2, sizeof(double));
		CreateBarFreqs();

		//
		int treble_buffer_size = GetTrebleBufferSize(rate);
		int FFTbassbufferSize = treble_buffer_size * 8;

		//
		input_buffer_size = FFTbassbufferSize * channels;
		input_buffer = (double*)calloc(input_buffer_size, sizeof(double));


		SetFFTInfo(bass_l, FFTbassbufferSize);    // BASS
		SetFFTInfo(mid_l, treble_buffer_size * 4);   // MID
		SetFFTInfo(treble_l, treble_buffer_size); // TREBLE

		if (channels == 2) {
			SetFFTInfo(bass_r, FFTbassbufferSize);    // BASS
			SetFFTInfo(mid_r, treble_buffer_size * 4);   // MID
			SetFFTInfo(treble_r, treble_buffer_size); // TREBLE
		}

		CreateHannMulBuffer();
	}


	void SpectrumVisual::Execute(float* in, int new_sample_count)
	{
		if (input_buffer == nullptr)
			return;

		/*	curt_frame++;
			if (curt_frame % skip_frame == 0)
				curt_frame = 0;
			else
				return;*/


		if (new_sample_count > input_buffer_size)
			new_sample_count = input_buffer_size;


		//往后偏移new_sample_count个数据，在开始处留出new_sample_count空位
		for (int n = input_buffer_size - 1; n >= new_sample_count; n--) {
			input_buffer[n] = input_buffer[n - new_sample_count];
		}

		//反向填充数据，整个数据在buffer中都是反向的
		for (int n = 0; n < new_sample_count; n++) {
			input_buffer[new_sample_count - n - 1] = in[n];
		}

		curt_frame++;
		if (curt_frame % skip_frame == 0)
			curt_frame = 0;
		else
			return;


		// fill the bass, mid and treble buffers
		FillInBuffer(bass_l, bass_r, bass_mul);
		FillInBuffer(mid_l, mid_r, mid_mul);
		FillInBuffer(treble_l, treble_r, treble_mul);

		//
		kiss_fftr(bass_l.state, bass_l.in, bass_l.out);
		kiss_fftr(mid_l.state, mid_l.in, mid_l.out);
		kiss_fftr(treble_l.state, treble_l.in, treble_l.out);

		//归一化
		NormFFTDatas(bass_l);
		NormFFTDatas(mid_l);
		NormFFTDatas(treble_l);

		if (channels == 2) {
			kiss_fftr(bass_r.state, bass_r.in, bass_r.out);
			kiss_fftr(mid_r.state, mid_r.in, mid_r.out);
			kiss_fftr(treble_r.state, treble_r.in, treble_r.out);

			//归一化
			NormFFTDatas(bass_r);
			NormFFTDatas(mid_r);
			NormFFTDatas(treble_r);
		}


		//寻找频带频率所对应的响应值
		int m, barFreq;
		float ratef = rate;
		float bass_n = ratef / bass_l.bufferSize; //bass_l的fft离散点中，频率增量间隔
		float mid_n = ratef / mid_l.bufferSize; //mid_l的fft离散点中，频率增量间隔
		float treble_n = ratef / treble_l.bufferSize; //treble_l的fft离散点中，频率增量间隔

		for (int i = 0; i < number_of_bars; i++)
		{
			barFreq = freq_of_bars[i];

			if (barFreq <= low_cut_off)
			{
				m = round(barFreq / bass_n); //barFreq频率所在的离散点位置
				amp_of_bars[i] = hypot(bass_l.out[m].r, bass_l.out[m].i);  //输出频率响应幅值

				if (channels == 2)
					amp_of_bars[i + number_of_bars] = hypot(bass_r.out[m].r, bass_r.out[m].i);
			}
			else if (barFreq <= high_cut_off)
			{
				m = round(barFreq / mid_n);
				amp_of_bars[i] = hypot(mid_l.out[m].r, mid_l.out[m].i);

				if (channels == 2)
					amp_of_bars[i + number_of_bars] = hypot(mid_r.out[m].r, mid_r.out[m].i);
			}
			else {
				m = round(barFreq / treble_n);
				amp_of_bars[i] = hypot(treble_l.out[m].r, treble_l.out[m].i);

				if (channels == 2)
					amp_of_bars[i + number_of_bars] = hypot(treble_r.out[m].r, treble_r.out[m].i);
			}
		}
	}

	void SpectrumVisual::FillInBuffer(FFTInfo& fftInfo_l, FFTInfo& fftInfo_r, double* mul)
	{
		for (uint16_t n = 0; n < fftInfo_l.bufferSize; n++) {
			if (channels == 2) {
				fftInfo_l.in[n] = input_buffer[n * 2] * mul[n];
				fftInfo_r.in[n] = input_buffer[n * 2 + 1] * mul[n];
			}
			else {
				fftInfo_l.in[n] = input_buffer[n] * mul[n];
			}
		}
	}


	//获取高音缓存尺寸
	int SpectrumVisual::GetTrebleBufferSize(int rate)
	{
		int treble_buffer_size = 128;

		if (rate > 8125 && rate <= 16250)
			treble_buffer_size *= 2;
		else if (rate > 16250 && rate <= 32500)
			treble_buffer_size *= 4;
		else if (rate > 32500 && rate <= 75000)
			treble_buffer_size *= 8;
		else if (rate > 75000 && rate <= 150000)
			treble_buffer_size *= 16;
		else if (rate > 150000 && rate <= 300000)
			treble_buffer_size *= 32;
		else if (rate > 300000)
			treble_buffer_size *= 64;

		return treble_buffer_size;
	}

	void SpectrumVisual::SetFFTInfo(FFTInfo& fftInfo, int fftBufferSize)
	{
		fftInfo.bufferSize = fftBufferSize;
		fftInfo.in = new double[fftBufferSize];
		fftInfo.out = new kiss_fft_cpx[fftBufferSize / 2 + 1];
		fftInfo.state = kiss_fftr_alloc(fftBufferSize, 0, 0, 0);
	}


	void SpectrumVisual::CreateHannMulBuffer()
	{
		//
		bass_mul = (double*)malloc(bass_l.bufferSize * sizeof(double));
		for (int i = 0; i < bass_l.bufferSize; i++)
			bass_mul[i] = 0.5 * (1 - cos(2 * M_PI * i / (bass_l.bufferSize - 1)));
		//
		mid_mul = (double*)malloc(mid_l.bufferSize * sizeof(double));
		for (int i = 0; i < mid_l.bufferSize; i++)
			mid_mul[i] = 0.5 * (1 - cos(2 * M_PI * i / (mid_l.bufferSize - 1)));

		//
		treble_mul = (double*)malloc(treble_l.bufferSize * sizeof(double));
		for (int i = 0; i < treble_l.bufferSize; i++)
			treble_mul[i] = 0.5 * (1 - cos(2 * M_PI * i / (treble_l.bufferSize - 1)));
	}

	//归一化fft数据
	void SpectrumVisual::NormFFTDatas(FFTInfo& fftInfo)
	{
		int count = fftInfo.bufferSize / 2 + 1;
		double invN = 1.0 / count;
		for (int n = 0; n < count; n++)
		{
			fftInfo.out[n].r *= invN;
			fftInfo.out[n].i *= invN;
		}
	}

	//生成频带频率
	//频段范围:20hz~20000hz, 求结束频率相对初始频率的倍率 mul = 20000/20，
	//按照频段数量number_of_bars，划分number_of_bars个频段，则每个频段之间的倍率为n = mul^(1/number_of_bars)
	// n*n*n*....*n 一共乘上number_of_bars过后正好是mul倍
	//初始频率20*(n*n*n*....*n)正好算的结束频率，则中间的其他频段也可按间隔求得，这就是以对数间隔量间隔的频率
	void SpectrumVisual::CreateBarFreqs()
	{
		freq_of_bars = new int[number_of_bars];
		double m = (double)end_freq / start_freq;
		double n = pow(m, 1.0f / (number_of_bars - 1));
		freq_of_bars[0] = start_freq;
		for (int i = 1; i < number_of_bars; i++)
			freq_of_bars[i] = start_freq * pow(n, i);

	}


	//生成smoothBars
	//smoothAmps: 输出的数据
	//minDB: 对频率的幅值响应限制再小于值60dB都为0值
	//interpCount:对两个频率幅值之间的插值个数
	//smoothCoffe:对整个数据的平滑系数，默认值:0.6
	//channel：需要对那个通道运算，0:左通道， 1:右通道，2:两个通道取最大值
	int SpectrumVisual::CreateSmoothAmpBars(
		double* smoothAmps,
		float minDB, int interpCount, float smoothCoffe,
		int channel)
	{
		int count = GetNumberOfBars();
		double val = 0;

		for (int i = 0; i < count; i++) {
			if (channel == 0 || channels != 2) //左通道
				val = amp_of_bars[i];
			else if (channel == 1 && channels == 2) //右通道
				val = amp_of_bars[i + count];
			else if (channels == 2) {
				val = max(amp_of_bars[i], amp_of_bars[i + count]);
			}

			float db = -GainToDecibels((float)val);
			val = 1 - min(minDB, db) / minDB;
			if (val < 0.01)val = 0.01f;
			else if (val > 1) val = 1;
			tmp_amp_of_bars[i] = val;
		}


		//对前后两个amp值进行插值，以平滑数据
		int n = 0;
		double a, b;
		a = smoothAmps[count - 1] = tmp_amp_of_bars[count - 1];
		smoothAmps[n] = a;

		for (int i = 0; i < count; i++) {
			b = tmp_amp_of_bars[i];
			val = (b - a) / interpCount;

			for (int j = 1; j <= interpCount; j++) {
				smoothAmps[n] = a + val * j;
				n++;
			}
			a = b;
		}

		SmoothData(smoothAmps, n, smoothCoffe);  //0.5

		return n;

	}

	// 增益转分贝
	float SpectrumVisual::GainToDecibels(float gain)
	{
		return gain <= 0.000001f ? -144 : (float)(20.0 * log10(gain));
	}


	//平滑数据
	void SpectrumVisual::SmoothData(double* data, int len, double smoothCoffe)
	{
		double gs = 0, gc;
		for (int i = 0; i < len; i++)
		{
			gc = data[i];
			if (i == 0) {
				gs = data[i] = gc;
			}
			else {
				gs = smoothCoffe * gs + (1 - smoothCoffe) * gc;
				data[i] = gs;
			}
		}
		//
		if (gs < data[0])
		{
			gs = data[0];
			for (int i = len - 1; i > len - 5; i--)
			{
				gs = smoothCoffe * gs + (1 - smoothCoffe) * data[i];
				data[i] = gs;
			}
		}
		else {
			for (int i = 0; i < 5; i++) {
				gs = smoothCoffe * gs + (1 - smoothCoffe) * data[i];
				data[i] = gs;
			}
		}
	}
}