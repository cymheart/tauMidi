#ifndef _MathUtils_h_
#define _MathUtils_h_

#include"Utils.h"
#include"Vec2.h"

namespace scutils
{

#define SINE_TABLE_SIZE 256
	extern double sine_table[SINE_TABLE_SIZE];
	double FastSin(double x);
	double FastCos(double x);

	//2^p
	inline float FastPow2(float p)
	{
		float offset = (p < 0) ? 1.0f : 0.0f;
		float clipp = (p < -126) ? -126.0f : p;
		int w = (int)clipp;

		float z = clipp - w + offset;
		union V { uint32_t i; float f; }v;
		v.i = (int)((1 << 23) * (clipp + 121.2740575f
			+ 27.7280233f / (4.84252568f - z)
			- 1.49012907f * z));
		return v.f;
	}

	/** From Musicdsp.org "Fast power and root estimates for 32bit floats)
	Original code by Stefan Stenzel
	These are approximations
	*/
	inline float FastPower(float f, int n)
	{
		if (f == 0)
			return 0;

		long* lp, l;
		lp = (long*)(&f);
		l = *lp;
		l -= 0x3F800000;
		l <<= (n - 1);
		l += 0x3F800000;
		*lp = l;
		return f;
	}


	inline float FastRoot(float f, int n)
	{
		long* lp, l;
		lp = (long*)(&f);
		l = *lp; l -= 0x3F800000l; l >>= (n - 1); l += 0x3F800000l;
		*lp = l;
		return f;
	}

	inline float Clampf(float in, float min, float max)
	{
		return fmin(fmax(in, min), max);
	}

	inline int Clamp(int in, int min_, int max_)
	{
		return min(max(in, min_), max_);
	}

	/// <summary>
	/// N = 3: P = (1-t)^2*P0 + 2*t*(1-t)*P1 + t^2*P2
	/// </summary>
	Vec2 SquareBezier(double t, Vec2 p0, Vec2 p1, Vec2 p2);

}

#endif
