#ifndef _SampleGenerator_h_
#define _SampleGenerator_h_

#include "TauTypes.h"

namespace tau
{
	//��������������
	//��������������ͨ�����ӵ������������ɵģ�
	//������ӿڿ����Զ�����������ͨ������������������̬����
	//by cymheart, 2022.
	class ZoneSampleGenerator
	{
	public:
		virtual ~ZoneSampleGenerator() {}
		virtual void Init(int key) = 0;
		virtual void Trigger(float velocity) = 0;
		virtual float Out(int prevSampleIntPos, int nextSampleIntPos, float a) = 0;


	};

	//����������,����������������̬����������
	class SampleGenerator
	{
	public:
		virtual ~SampleGenerator() {}
		virtual ZoneSampleGenerator* CreateZoneSampleGen(int key) = 0;
		virtual void ReleaseZoneSampleGen(ZoneSampleGenerator* sampleGen) = 0;
		virtual float GetSampleRate() = 0;
	};
}

#endif
