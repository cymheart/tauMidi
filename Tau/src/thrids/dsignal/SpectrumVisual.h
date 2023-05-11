#ifndef _SpectrumVisual_h_
#define _SpectrumVisual_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include <kissfft/kiss_fftr.h>

using namespace scutils;

namespace dsignal
{

	struct FFTInfo {
		int bufferSize = 0;
		kiss_fftr_cfg state = nullptr;
		double* in = nullptr;
		kiss_fft_cpx* out = nullptr;
	};

	//频谱可视化
	class SpectrumVisual
	{
	public:
		~SpectrumVisual();

		void Init(
			int number_of_bars = 40, int start_freq = 30, int end_freq = 21000, unsigned int rate = 44100,
			int channels = 2, int low_cut_off = 160, int high_cut_off = 5120, int skip_frame = 3);

		void Execute(float* in, int new_sample_count);

		//获取通道数量
		int GetChannels() {
			return channels;
		}

		//获取频率条的个数
		int GetNumberOfBars() {
			return number_of_bars;
		}

		//获取频率条的频率
		int32_t* GetFreqOfBars() {
			return freq_of_bars;
		}

		//获取频带频率的幅值(响应值，归一化)
		double* GetAmpOfBars() {
			return amp_of_bars;
		}

		//生成smoothBars
		//smoothAmps: 输出的数据
		//minDB: 对频率的幅值响应限制再小于值60dB都为0值
		//interpCount:对两个频率幅值之间的插值个数
		//smoothCoffe:对整个数据的平滑系数，默认值:0.6
		//channel：需要对那个通道运算，0:左通道， 1:右通道，2:两个通道取最大值
		int CreateSmoothAmpBars(
			double* smoothAmps,
			float minDB = 60, int interpCount = 4, float smoothCoffe = 0.6f,
			int channel = 2);

	protected:

		void FreeDatas();
		void FreeFFTInfo(FFTInfo& fftInfo);
		//获取高音缓存尺寸
		int GetTrebleBufferSize(int rate);
		void SetFFTInfo(FFTInfo& fftInfo, int fftBufferSize);
		size_t ComputeKissFFTStateMemorySize(int nfft);
		void FillInBuffer(FFTInfo& fftInfo_l, FFTInfo& fftInfo_r, double* mul);
		void CreateHannMulBuffer();
		//归一化fft数据
		void NormFFTDatas(FFTInfo& fftInfo);
		void CreateBarFreqs();

		// 增益转分贝
		float GainToDecibels(float gain);
		//平滑数据
		void SmoothData(double* data, int len, double smoothCoffe);

	private:

		int number_of_bars = 0;
		int start_freq = 0;
		int end_freq = 0;
		int channels = 0;
		unsigned int rate = 0;
		int low_cut_off = 160;
		int high_cut_off = 5120;
		int input_buffer_size = 0;
		int skip_frame = 0;

		//当前帧
		int curt_frame = 0;

		//频率条的频率
		int32_t* freq_of_bars = nullptr;
		//频带频率的幅值(响应值，归一化)
		double* amp_of_bars = nullptr;
		double* tmp_amp_of_bars = nullptr;

		double* input_buffer = nullptr;

		double* bass_mul = nullptr;
		double* mid_mul = nullptr;
		double* treble_mul = nullptr;

		FFTInfo bass_l, bass_r;
		FFTInfo mid_l, mid_r;
		FFTInfo treble_l, treble_r;


	};

}

#endif