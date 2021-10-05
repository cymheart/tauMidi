#include"Overdrive.h"

namespace tauFX
{
	Overdrive::Overdrive(Synther* synther)
		:TauEffect(synther)
	{
		leftOverdrive = new daisysp::Overdrive();
		rightOverdrive = new daisysp::Overdrive();

		leftOverdrive->Init();
		rightOverdrive->Init();
	}

	Overdrive::~Overdrive()
	{
		DEL(leftOverdrive);
		DEL(rightOverdrive);
	}

	/** Set the amount of drive
		\param drive Works from 0-1
	*/
	void Overdrive::SetDrive(float drive)
	{
		leftOverdrive->SetDrive(drive);
		rightOverdrive->SetDrive(drive);
	}


	void Overdrive::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = leftOverdrive->Process(leftChannelSamples[i]);
			rightChannelSamples[i] = leftOverdrive->Process(rightChannelSamples[i]);
		}
	}
}
