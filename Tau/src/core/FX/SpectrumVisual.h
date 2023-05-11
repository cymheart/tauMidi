#ifndef _FxSpectrumVisual_h_
#define _FxSpectrumVisual_h_

#include "scutils/Utils.h"
#include"TauEffect.h"
#include <dsignal/SpectrumVisual.h>


namespace tauFX
{
	/*频谱可视化*/
	class DLL_CLASS SpectrumVisual : public TauEffect
	{
	public:
		SpectrumVisual();
		~SpectrumVisual();

		int GetChannels();

		//频谱可视初始化
		void Init(
			int number_of_bars = 40,
			unsigned int rate = 44100, int channels = 2,
			int start_freq = 30, int end_freq = 21000,
			int low_cut_off = 160, int high_cut_off = 5120,
			int skip_frame = 3);

		//获取采样流的频谱的频带条
		int GetAmpBars(double* ampOfBars, int32_t* freqOfBars);

		//生成smoothBars频带条
		//smoothAmps: 输出的数据
		//minDB: 对频率的幅值响应限制再小于值60dB都为0值
		//interpCount:对两个频率幅值之间的插值个数
		//smoothCoffe:对整个数据的平滑系数，默认值:0.6
		//channel：需要对那个通道运算，0:左通道， 1:右通道，2:两个通道取最大值
		int CreateSmoothAmpBars(
			double* smoothAmps,
			float minDB = 60, int interpCount = 4, float smoothCoffe = 0.6,
			int channel = 2);

	private:
		void EffectProcess(float* synthStream, int numChannels, int channelSampleCount);

	private:
		dsignal::SpectrumVisual spectrumVisual;
	};
}

#endif