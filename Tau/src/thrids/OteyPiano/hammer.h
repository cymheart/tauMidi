#ifndef HAMMER_H
#define HAMMER_H
#include <stdio.h>
#include"DWG.h"


namespace oteypiano
{

	struct Integrator {
		float lastout;
		float iFs;
		float go(float in) {
			float out = lastout + in * iFs;
			lastout = out;
			return out;
		}
		void Init(float Fs, float inival = 0.0) { lastout = inival; iFs = 1.0 / Fs; }
	};

	struct UnitDelay {
		float lastin;
		float go(float in) {
			float out = lastin;
			lastin = in;
			return out;
		}
		void Init() { lastin = 0.0; }
	};

	class Hammer {
	public:
		Hammer(float f, float Fs, float m, float K, float p, float Z, float alpha, float v0);
		virtual ~Hammer() = default;


		void* operator new(size_t sz) {
			return malloc(sz);
		}
		void operator delete(void* pObject) {
			free((Hammer*)pObject);
		}
		virtual float load(float vin) = 0;
		virtual void trigger(float v) = 0;

		float a;
		//float v0;
		float mi;
		float K;
		float p;
		float Fs;
		float F;

		float Z2i;

	};

	class BanksHammer :public Hammer
	{
	public:
		BanksHammer(float f, float Fs, float m, float K, float p, float Z, float alpha, float v0);
		float load(float vin);
		void trigger(float v);
		float vh;
		float oldvin;
		Integrator intv;
		Integrator intvh;
		UnitDelay undel;
	};

	class StulovHammer :public Hammer
	{
	public:
		StulovHammer(float f, float Fs, float m, float K, float p, float Z, float alpha, float v0);
		float load(float vin);
		void trigger(float v);
		float upprev;
		float alpha;
		float x;
		float v;
		int S;
		float dt;
		float dti;
	};

}

#endif
