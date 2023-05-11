#include"Mp3Recorder.h"

namespace scutils
{
	lame_global_flags* Mp3Recorder::glf = NULL;

	/**
   * 初始化 lame编码器
   * @param inSampleRate 输入采样率
   * @param outChannel 声道数
   * @param outSampleRate 输出采样率
   * @param outBitrate 比特率(kbps)
   * @param quality 0~9，0最好
   */
	void Mp3Recorder::Init(
		int inSamplerate, int outChannel, int outSamplerate,
		int outBitrate, int quality)
	{
		if (glf != NULL) {
			lame_close(glf);
			glf = NULL;
		}

		glf = lame_init();
		lame_set_in_samplerate(glf, inSamplerate);
		lame_set_num_channels(glf, outChannel);
		lame_set_out_samplerate(glf, outSamplerate);
		lame_set_brate(glf, outBitrate);
		lame_set_quality(glf, quality);
		lame_init_params(glf);
	}

	/**
	 *  编码，把录制的 PCM 数据转换成 mp3 格式
	 *
	 * @param buffer_l 左声道输入数据
	 * @param buffer_r 右声道输入数据
	 * @param nsamples   输入数据的size输入数据的size
	 * @param mp3buf 输出数据
	 * @return 输出到mp3buf的byte数量
	 */
	int Mp3Recorder::Encode(short* buffer_l, short* buffer_r, int nsamples, uint8_t* mp3buf, int mp3bufSize)
	{
		return lame_encode_buffer(glf, buffer_l, buffer_r, nsamples, mp3buf, mp3bufSize);
	}


	/**
	 *  编码，把  录制的 PCM 数据转换成 mp3 格式
	 *
	 * @param buffer_l 左声道输入数据
	 * @param buffer_r 右声道输入数据
	 * @param nsamples   输入数据的size
	 * @param mp3buf 输出数据
	 * @return 输出到mp3buf的byte数量
	 */
	int Mp3Recorder::EncodeFloat(float* buffer_l, float* buffer_r, int nsamples, uint8_t* mp3buf, int mp3bufSize)
	{
		return lame_encode_buffer_ieee_float(glf, buffer_l, buffer_r, nsamples, mp3buf, mp3bufSize);
	}


	/**
	 *  刷写
	 *
	 * @param mp3buf mp3数据缓存区
	 * @return  返回刷写的数量
	 */
	int Mp3Recorder::flush(uint8_t* mp3buf, int mp3bufSize)
	{
		return lame_encode_flush(glf, mp3buf, mp3bufSize);
	}


	/**
* 关闭 lame 编码器，释放资源
*/
	void Mp3Recorder::close()
	{
		lame_close(glf);
		glf = NULL;
	}

}