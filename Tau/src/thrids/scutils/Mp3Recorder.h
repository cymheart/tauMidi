#ifndef _Mp3Recorder_h_
#define _Mp3Recorder_h_

#include "libmp3lame/lame.h"
#include "Utils.h"

namespace scutils
{

	class Mp3Recorder
	{
	public:
		/**
   * ��ʼ�� lame������
   * @param inSampleRate ���������
   * @param outChannel ������
   * @param outSampleRate ���������
   * @param outBitrate ������(kbps)
   * @param quality 0~9��0���
   */
		void Init(int inSamplerate, int outChannel, int outSamplerate, int outBitrate, int quality);

		/**
	  *  ���룬��¼�Ƶ� PCM ����ת���� mp3 ��ʽ
	  *
	  * @param buffer_l ��������������
	  * @param buffer_r  ��������������
	  * @param nsamples �������ݵ�size
	  * @param mp3buf�������
	  * @return �����mp3buf��byte����
	  */
		int Encode(short* buffer_l, short* buffer_r, int nsamples, uint8_t* mp3buf, int mp3bufSize);

		/**
		 *  ���룬��¼�Ƶ� PCM ����ת���� mp3 ��ʽ
		 *
		 * @param buffer_l ��������������
		 * @param buffer_r ��������������
		 * @param nsamples   �������ݵ�size
		 * @param mp3buf �������
		 * @return �����mp3buf��byte����
		 */
		int EncodeFloat(float* buffer_l, float* buffer_r, int nsamples, uint8_t* mp3buf, int mp3bufSize);


		/**
 *  ˢд
 *
 * @param mp3buf mp3���ݻ�����
 * @return  ����ˢд������
 */
		int flush(uint8_t* mp3buf, int mp3bufSize);


		/**
	* �ر� lame ���������ͷ���Դ
	*/
		void close();

	private:
		static lame_global_flags* glf;
	};
}

#endif