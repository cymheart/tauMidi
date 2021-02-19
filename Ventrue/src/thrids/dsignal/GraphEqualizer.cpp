#include"GraphEqualizer.h"

namespace dsignal
{
	GraphEqualizer::GraphEqualizer()
	{
		FreqBandEQInfo freqbandEQInfos[10];
		freqbandEQInfos[0].freqStart = 22;
		freqbandEQInfos[0].freqEnd = 44;
		freqbandEQInfos[0].gainDB = 0;

		for (int i = 1; i < 10; i++)
		{
			freqbandEQInfos[i].freqStart = freqbandEQInfos[i - 1].freqEnd;
			freqbandEQInfos[i].freqEnd = freqbandEQInfos[i].freqStart * 2;
			freqbandEQInfos[i].gainDB = 0;
		}

		freqbandEQInfos[9].freqEnd = 19912;

		SetFreqBandEQInfos(freqbandEQInfos, 10);
		Create();
	}

	GraphEqualizer::~GraphEqualizer()
	{
		Clear();
	}

	void GraphEqualizer::Clear()
	{
		if (freqBandEQInfo != nullptr) {
			free(freqBandEQInfo);
			freqBandEQInfo = nullptr;
		}

		if (freqBandBiquad != nullptr) {
			delete[] freqBandBiquad;
			freqBandBiquad = nullptr;
		}

		size = 0;
	}

	vector<dsignal::Filter*> GraphEqualizer::GetFilters()
	{
		vector<dsignal::Filter*> biquads;
		for (int i = 0; i < size * 2; i++)
		{
			biquads.push_back(&(freqBandBiquad[i]));
		}

		return biquads;
	}


	void GraphEqualizer::SetFreqBandEQInfos(FreqBandEQInfo* freqBandEQInfos, int size)
	{
		if (size <= 0 || freqBandEQInfos == nullptr)
			return;

		try
		{
			Clear();
			freqBandEQInfo = (FreqBandEQInfo*)malloc(sizeof(FreqBandEQInfo) * size);
			memcpy(freqBandEQInfo, freqBandEQInfos, (size_t)(size * sizeof(FreqBandEQInfo)));
			freqBandBiquad = new Biquad[size * 2];
			this->size = size;
		}
		catch (exception)
		{

		}
	}


	void GraphEqualizer::Create()
	{
		int j = 0;
		for (int i = 0; i < size; i++)
		{
			//
			freqBandBiquad[j].biquadFilterType = BiquadFilterType::HighShelf;
			freqBandBiquad[j].qtype = QType::ShelfSlope;
			freqBandBiquad[j].f0 = freqBandEQInfo[i].freqStart;
			freqBandBiquad[j].fs = sampleRate;
			freqBandBiquad[j].S = 1;
			freqBandBiquad[j].gainDB = freqBandEQInfo[i].gainDB;
			freqBandBiquad[j].CalculateCoefficients();
			j++;

			freqBandBiquad[j].biquadFilterType = BiquadFilterType::HighShelf;
			freqBandBiquad[j].qtype = QType::ShelfSlope;
			freqBandBiquad[j].f0 = freqBandEQInfo[i].freqEnd;
			freqBandBiquad[j].fs = sampleRate;
			freqBandBiquad[j].S = 1;
			freqBandBiquad[j].gainDB = -freqBandEQInfo[i].gainDB;
			freqBandBiquad[j].CalculateCoefficients();
			j++;
		}
	}

	void GraphEqualizer::SetFreqBandGain(int bandIdx, float gainDB)
	{
		if (bandIdx < 0 || bandIdx >= size)
			return;

		if (freqBandEQInfo[bandIdx].gainDB == gainDB)
			return;

		freqBandEQInfo[bandIdx].gainDB = gainDB;

		int idx = bandIdx * 2;
		freqBandBiquad[idx].gainDB = gainDB;
		freqBandBiquad[idx].CalculateCoefficients();

		freqBandBiquad[idx + 1].gainDB = -gainDB;
		freqBandBiquad[idx + 1].CalculateCoefficients();
	}


	double GraphEqualizer::Filtering(double input)
	{
		for (int i = 0; i < size * 2; i++)
		{
			input = freqBandBiquad[i].Filtering(input);
		}

		return input;
	}

}
