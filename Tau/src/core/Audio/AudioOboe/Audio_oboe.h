#ifdef __ANDROID__
#ifndef VENTRUE_AUDIO_OBOE_H
#define VENTRUE_AUDIO_OBOE_H

#include <oboe/Oboe.h>
#include "Audio/Audio.h"
namespace tau
{
	class Audio_oboe :public Audio {

	public:
		Audio_oboe();
		~Audio_oboe();
		virtual void Close();
		virtual void Open();

	private:
		class MyCallback : public oboe::AudioStreamCallback {
		public:
			Audio_oboe* audio_oboe;
			oboe::DataCallbackResult
				onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames);
		};

	private:
		MyCallback myCallback;
		//oboe::ManagedStream managedStream = oboe::ManagedStream();

		oboe::AudioStreamBuilder mBuilder;
		oboe::AudioStream *mStream = nullptr;
	};

}


#endif //VENTRUE_AUDIO_OBOE_H
#endif
