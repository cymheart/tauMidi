#ifdef __ANDROID__
#include "Audio_oboe.h"
#include"scutils/logging_macros.h"

namespace ventrue
{
	Audio_oboe::Audio_oboe()
	{
		myCallback.audio_oboe = this;
		managedStream = oboe::ManagedStream();
	}

	oboe::DataCallbackResult Audio_oboe::MyCallback::
		onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames)
	{
		// 需要生成 AudioFormat::Float 类型数据 , 该缓冲区类型也是该类型
		// 生产者需要检查该格式
		// oboe::AudioStream *audioStream 已经转换为适当的类型
		// 获取音频数据缓冲区
		audio_oboe->audioCallback(audio_oboe->data, (uint8_t*)audioData, numFrames * 4 * audio_oboe->channelCount);

		return oboe::DataCallbackResult::Continue;
	}


	Audio_oboe::~Audio_oboe()
	{
		managedStream->close();
	}

	void Audio_oboe::Open()
	{
		// 1. 音频流构建器
		oboe::AudioStreamBuilder  builder;
		// 设置音频流方向
		builder.setDirection(oboe::Direction::Output);
		// 设置性能优先级
		builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
		// 设置共享模式 , 独占
		builder.setSharingMode(oboe::SharingMode::Exclusive);
		// 设置音频采样格式
		builder.setFormat(oboe::AudioFormat::Float);
		// 设置声道数 , 单声道/立体声
		builder.setChannelCount(channelCount);
		// 设置采样率
		builder.setSampleRate(freq);

		//设置固定帧数,注意此处必须设为固定帧数，ventrue合成器引擎只支持固定帧数的处理
		//否者会合成不正确样本
		builder.setFramesPerDataCallback(sampleCount);

		// 设置回调对象 , 注意要设置 AudioStreamCallback * 指针类型
		builder.setCallback(&myCallback);

		// 2. 通过 AudioStreamBuilder 打开 Oboe 音频流
		oboe::Result result = builder.openManagedStream(managedStream);
		LOGI("openManagedStream result : %s", oboe::convertToText(result));

		// 3. 开始播放
		result = managedStream->requestStart();
		LOGI("requestStart result : %s", oboe::convertToText(result));

	}
}
#endif
