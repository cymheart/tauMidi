#ifndef _PIANO_H_
#define _PIANO_H_


#define NUM_NOTES 128

#include "filter.h"
#include "dwgs.h"
#include "reverb.h"
#include "hammer.h"

namespace oteypiano
{

	typedef struct
	{
		float f;
		float Fs;
		float velocity;
		float minr;
		float maxr;
		float amprl;
		float amprr;
		float mult_radius_core_string;
		float minL;
		float maxL;
		float ampLl;
		float ampLr;
		float mult_density_string;
		float mult_modulus_string;
		float mult_impedance_bridge;
		float mult_impedance_hammer;
		float mult_mass_hammer;
		float mult_force_hammer;
		float mult_hysteresis_hammer;
		float mult_stiffness_exponent_hammer;
		float position_hammer;
		float mult_loss_filter;
		float detune;
		int hammer_type;
	}PianoKeyParams;

	typedef struct
	{
		float eq1, eq2, eq3;
		float c1, c3;
	}PianoSoundboardParams;


	struct DWGReverb {
		FilterC1C3 decay[8];
		CircularBuffer2POWSizedT<1024> delay[8];
		void setcoeffs(float c1, float c3, float a, float mix, float Fs);
		float tick(float in);
		float mix;
		float A[8][8];
		float o[8];
		float b[8];
		float c[8];
		static const int lengths[8];//= {37,87,181,271,359,593,688,721};
	};



	class PianoSoundBoard
	{
	public:
		void Init(PianoSoundboardParams& param);
		void Go(float* in, float* out, int count);

		PianoSoundboardParams param;
		DWGReverb reverb;
		Biquad shaping1;
		Biquad shaping2;
		Biquad shaping3;
	};


	class PianoKey
	{
	public:

		PianoKey();
		~PianoKey();

		void Clear();
		void Init(PianoKeyParams& param);
		void Trigger(float v);
		long Go(float* out, int samples);

		void* operator new(size_t sz) {
			return malloc(sizeof(PianoKey));
		}
		void operator delete(void* pObject) {
			free(pObject);
		}

		float v0 = 0;
		float Fs = 0;
		float Z = 0;
		float Zb = 0;
		float Zh = 0;
		float f = 0;

		int nstrings = 0;
		dwgs* string[3] = { nullptr };
		Hammer* hammer = { nullptr };
	};


}

#endif
