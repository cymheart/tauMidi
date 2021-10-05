#ifdef _WIN32

#include"Audio_Rt.h"

namespace tau
{
	Audio_Rt::~Audio_Rt()
	{
		Close();
	}
	void Audio_Rt::Close()
	{
		if (!isOpened)
			return;

		isOpened = false;
		dac->stopStream();
		dac->closeStream();
		delete dac;
		dac = nullptr;
	}

	void Audio_Rt::Open()
	{
		if (isOpened)
			return;

		dac = new RtAudio(RtAudio::Api::WINDOWS_DS);

		if (dac->getDeviceCount() < 1) {
			std::cout << "\nNo audio devices found!\n";
			return;
		}

		// Let RtAudio print messages to stderr.
		dac->showWarnings(true);

		// Set our stream parameters for output only.
		RtAudio::StreamParameters oParams;
		oParams.nChannels = channelCount;
		oParams.firstChannel = 0;
		oParams.deviceId = dac->getDefaultOutputDevice();

		RtAudio::StreamOptions options;
		options.flags = RTAUDIO_HOG_DEVICE;
		options.flags |= RTAUDIO_SCHEDULE_REALTIME;
		//#if !defined( USE_INTERLEAVED )
		//		options.flags |= RTAUDIO_NONINTERLEAVED;
		//#endif
		try {
			dac->openStream(
				&oParams, NULL, RTAUDIO_FLOAT32, freq,
				&sampleCount, &output, (void*)this, &options);
			dac->startStream();
			isOpened = true;
			return;
		}
		catch (RtAudioError& e) {
			e.printMessage();
			goto cleanup;
		}



	cleanup:
		if (dac->isStreamOpen())
			dac->closeStream();
	}

	int Audio_Rt::output(void* outputBuffer, void* /*inputBuffer*/, unsigned int nBufferFrames,
		double /*streamTime*/, RtAudioStreamStatus /*status*/, void* data)
	{
		Audio_Rt* rt = (Audio_Rt*)data;
		if (!rt->isOpened)
			return 1;

		rt->audioCallback(rt->data, (uint8_t*)outputBuffer, nBufferFrames * sizeof(float) * rt->channelCount);
		return 0;
	}
}

#endif
