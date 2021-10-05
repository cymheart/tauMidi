#ifdef _WIN32

#include"Audio_pa.h"

namespace tau
{

	Audio_pa::~Audio_pa()
	{
		Close();
	}

	void Audio_pa::Close()
	{
		if (!isOpened)
			return;

		isOpened = false;
		PaError err;

		err = Pa_StopStream(stream);
		if (err != paNoError) goto error;

		err = Pa_CloseStream(stream);
		if (err != paNoError)goto error;
		return;

	error:
		Pa_Terminate();
		fprintf(stderr, "An error occurred while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		fprintf(stderr, "Host Error message: %s\n", Pa_GetLastHostErrorInfo()->errorText);
	}

	//void Audio_pa::Open()
	//{
	//	PaStreamParameters outputParameters;
	//	//PaWinDirectSoundStreamInfo dsoundStreamInfo;

	//	PaError err = Pa_Initialize();
	//	//int deviceIndex;
	//	if (err != paNoError)
	//		goto error;

	//	//deviceIndex = Pa_GetHostApiInfo(Pa_HostApiTypeIdToHostApiIndex(paDirectSound))->defaultOutputDevice;
	//	//outputParameters.device = deviceIndex; // Pa_GetDefaultOutputDevice(); /* default output device */

	//	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	//	if (outputParameters.device == paNoDevice)
	//		goto error;

	//	outputParameters.channelCount = channelCount;                      /* stereo output */
	//	outputParameters.sampleFormat = paFloat32;               /* 32 bit floating point output */
	//	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	//	outputParameters.hostApiSpecificStreamInfo = NULL;


	//	/*	dsoundStreamInfo.size = sizeof(PaWinDirectSoundStreamInfo);
	//		dsoundStreamInfo.hostApiType = paDirectSound;
	//		dsoundStreamInfo.version = 2;
	//		dsoundStreamInfo.flags = paWinDirectSoundUseLowLevelLatencyParameters;
	//		dsoundStreamInfo.framesPerBuffer = 4096;
	//		outputParameters.hostApiSpecificStreamInfo = &dsoundStreamInfo;*/
	//		/*if (Pa_IsFormatSupported(0, &outputParameters, freq) == paFormatIsSupported) {
	//			printf("Pa_IsFormatSupported reports device will support %d channels.\n", channelCount);
	//		}
	//		else {
	//			printf("Pa_IsFormatSupported reports device will not support %d channels.\n", channelCount);
	//		}*/

	//	err = Pa_OpenStream(
	//		&stream,
	//		NULL,                         /* no input */
	//		&outputParameters,
	//		freq,
	//		sampleCount,                   /* frames per buffer */
	//		paNoFlag,
	//		patest1Callback,
	//		this);

	//	//err = Pa_OpenDefaultStream(
	//	//	&stream,
	//	//	0,                         /* no input */
	//	//	2,
	//	//	paFloat32,
	//	//	freq,
	//	//	sampleCount,                   /* frames per buffer */
	//	//	patest1Callback,
	//	//	this);

	//	if (err != paNoError)
	//		goto error;


	//	err = Pa_StartStream(stream);
	//	if (err != paNoError) goto error;

	//	isOpened = true;
	//	//printf("Waiting for sound to finish.\n");
	//	return;

	//error:
	//	Pa_Terminate();
	//	fprintf(stderr, "An error occurred while using the portaudio stream\n");
	//	fprintf(stderr, "Error number: %d\n", err);
	//	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
	//	fprintf(stderr, "Host Error message: %s\n", Pa_GetLastHostErrorInfo()->errorText);

	//}

	void Audio_pa::Open()
	{
		if (isOpened)
			return;

		PaStreamParameters outputParameters;
		PaWinDirectSoundStreamInfo dsoundStreamInfo;

		PaError err = Pa_Initialize();
		int deviceIndex;
		if (err != paNoError)
			goto error;

		deviceIndex = Pa_GetHostApiInfo(Pa_HostApiTypeIdToHostApiIndex(paDirectSound))->defaultOutputDevice;
		outputParameters.device = deviceIndex; // Pa_GetDefaultOutputDevice(); /* default output device */

		outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
		if (outputParameters.device == paNoDevice)
			goto error;

		outputParameters.channelCount = channelCount;                      /* stereo output */
		outputParameters.sampleFormat = paFloat32;               /* 32 bit floating point output */
		outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
		outputParameters.hostApiSpecificStreamInfo = NULL;


		dsoundStreamInfo.size = sizeof(PaWinDirectSoundStreamInfo);
		dsoundStreamInfo.hostApiType = paDirectSound;
		dsoundStreamInfo.version = 2;
		dsoundStreamInfo.flags = paWinDirectSoundUseLowLevelLatencyParameters;
		dsoundStreamInfo.framesPerBuffer = 4096;
		outputParameters.hostApiSpecificStreamInfo = &dsoundStreamInfo;
		if (Pa_IsFormatSupported(0, &outputParameters, freq) == paFormatIsSupported) {
			printf("Pa_IsFormatSupported reports device will support %d channels.\n", channelCount);
		}
		else {
			printf("Pa_IsFormatSupported reports device will not support %d channels.\n", channelCount);
		}

		err = Pa_OpenStream(
			&stream,
			NULL,                         /* no input */
			&outputParameters,
			freq,
			sampleCount,                   /* frames per buffer */
			paNoFlag,
			patest1Callback,
			this);

		//err = Pa_OpenDefaultStream(
		//	&stream,
		//	0,                         /* no input */
		//	2,
		//	paFloat32,
		//	freq,
		//	sampleCount,                   /* frames per buffer */
		//	patest1Callback,
		//	this);

		if (err != paNoError)
			goto error;


		err = Pa_StartStream(stream);
		if (err != paNoError) goto error;

		isOpened = true;
		//printf("Waiting for sound to finish.\n");
		return;

	error:
		Pa_Terminate();
		fprintf(stderr, "An error occurred while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		fprintf(stderr, "Host Error message: %s\n", Pa_GetLastHostErrorInfo()->errorText);

	}

	int Audio_pa::patest1Callback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData)
	{
		Audio_pa* pa = (Audio_pa*)userData;
		if (!pa->isOpened)
			return paComplete;

		pa->audioCallback(pa->data, (uint8_t*)outputBuffer, framesPerBuffer * sizeof(float) * pa->channelCount);
		return paContinue;
	}
}

#endif
