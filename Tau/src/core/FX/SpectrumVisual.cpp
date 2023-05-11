#include"SpectrumVisual.h"
#include <fstream>


namespace tauFX
{
	SpectrumVisual::SpectrumVisual()
	{

	}

	SpectrumVisual::~SpectrumVisual()
	{

	}

	int SpectrumVisual::GetChannels()
	{
		return spectrumVisual.GetChannels();
	}

	void SpectrumVisual::Init(int number_of_bars, unsigned int rate,
		int channels, int start_freq, int end_freq,
		int low_cut_off, int high_cut_off, int skip_frame)
	{
		spectrumVisual.Init(number_of_bars, start_freq, end_freq, rate,
			channels, low_cut_off, high_cut_off, skip_frame);
	}

	void SpectrumVisual::EffectProcess(float* synthStream, int numChannels, int channelSampleCount)
	{
		spectrumVisual.Execute(synthStream, channelSampleCount * numChannels);
	}


	int SpectrumVisual::GetAmpBars(double* ampOfBars, int32_t* freqOfBars)
	{
		int count = spectrumVisual.GetNumberOfBars();
		int32_t* freqs = spectrumVisual.GetFreqOfBars();
		double* amps = spectrumVisual.GetAmpOfBars();
		int ampsCount = spectrumVisual.GetChannels() * count;

		memcpy(freqOfBars, freqs, count * sizeof(int32_t));
		memcpy(ampOfBars, amps, ampsCount * sizeof(double));

		return count;
	}


	int SpectrumVisual::CreateSmoothAmpBars(
		double* smoothAmps,
		float minDB, int interpCount, float smoothCoffe,
		int channel)
	{

		return spectrumVisual.CreateSmoothAmpBars(
			smoothAmps, minDB,
			interpCount, smoothCoffe, channel);
	}

}