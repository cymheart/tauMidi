#include"GraphEqualizer.h"

namespace dsignal
{
	GraphEqualizer::GraphEqualizer()
	{
		/*	float info[] = {
				32,21, 63,42, 125,85, 250,169,
				500,340, 1000,679, 2000,1357, 4000,2716,
				8000,5430, 16000,10861 };*/

		float info[] = {
			32,1, 63,1, 125,1, 250,1,
			500,1, 1000,1, 2000,1, 4000,1,
			8000,1, 16000,1 };


		bandCount = 10;
		double coeff[6];
		int j = 0;

		for (int i = 0; i < bandCount; i++)
		{
			freqBandEQInfo[i].centerFreq = info[i * 2];
			freqBandEQInfo[i].bandWidth = info[i * 2 + 1];
			freqBandEQInfo[i].gainDB = 0;

			//
			freqBandBiquad[i].setup(sampleRate, info[i * 2], 0, info[i * 2 + 1]);

			j = 0;
			coeff[j++] = freqBandBiquad[i].getB0();
			coeff[j++] = freqBandBiquad[i].getB1();
			coeff[j++] = freqBandBiquad[i].getB2();
			coeff[j++] = freqBandBiquad[i].getA0();
			coeff[j++] = freqBandBiquad[i].getA1();
			coeff[j++] = freqBandBiquad[i].getA2();
			bandFilter[i].SetCoefficient(coeff, 6);
		}
	}

	vector<dsignal::Filter*> GraphEqualizer::GetFilters()
	{
		vector<dsignal::Filter*> filters;
		for (int i = 0; i < bandCount; i++)
		{
			filters.push_back(&(bandFilter[i]));
		}

		return filters;
	}


	void GraphEqualizer::SetFreqBandGain(int bandIdx, float gainDB)
	{
		if (bandIdx < 0 || bandIdx >= bandCount)
			return;

		if (freqBandEQInfo[bandIdx].gainDB == gainDB)
			return;

		freqBandEQInfo[bandIdx].gainDB = gainDB;

		freqBandBiquad[bandIdx].setup(
			sampleRate,
			freqBandEQInfo[bandIdx].centerFreq,
			freqBandEQInfo[bandIdx].gainDB,
			freqBandEQInfo[bandIdx].bandWidth);

		int j = 0;
		double coeff[6];
		coeff[j++] = freqBandBiquad[bandIdx].getB0();
		coeff[j++] = freqBandBiquad[bandIdx].getB1();
		coeff[j++] = freqBandBiquad[bandIdx].getB2();
		coeff[j++] = freqBandBiquad[bandIdx].getA0();
		coeff[j++] = freqBandBiquad[bandIdx].getA1();
		coeff[j++] = freqBandBiquad[bandIdx].getA2();
		bandFilter[bandIdx].SetCoefficient(coeff, 6);
	}


	double GraphEqualizer::Filtering(double input)
	{
		for (int i = 0; i < bandCount; i++)
		{
			input = freqBandBiquad[i].filter(input);
		}

		return input;
	}

}
