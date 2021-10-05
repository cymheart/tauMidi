#ifndef _EffectOverdrive_h_
#define _EffectOverdrive_h_

#include <DaisySP/daisysp.h>
#include"TauEffect.h"

namespace tauFX
{
	class DLL_CLASS Overdrive : public TauEffect
	{
	public:
		Overdrive(Synther* synther = nullptr);
		virtual ~Overdrive();

		/** Set the amount of drive
		\param drive Works from 0-1
	*/
		void SetDrive(float drive);

	public:
		/** Set the amount of drive
			\param drive Works from 0-1
		*/
		void SetDriveTask(float drive);

	private:
		static void _SetDriveTask(Task* task);


	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		daisysp::Overdrive* leftOverdrive;
		daisysp::Overdrive* rightOverdrive;
	};
}
#endif
