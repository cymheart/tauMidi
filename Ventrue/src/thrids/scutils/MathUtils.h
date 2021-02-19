#ifndef _MathUtils_h_
#define _MathUtils_h_

#include"Utils.h"
#include"Vec2.h"

namespace scutils
{

#define SINE_TABLE_SIZE 256
	extern double sine_table[SINE_TABLE_SIZE];
	DLL_FUNC double FastSin(double x);
	DLL_FUNC double FastCos(double x);
	DLL_FUNC float FastPow2(float p);

	/// <summary>
	/// N = 3: P = (1-t)^2*P0 + 2*t*(1-t)*P1 + t^2*P2
	/// </summary>
	DLL_FUNC Vec2 SquareBezier(double t, Vec2 p0, Vec2 p1, Vec2 p2);
}

#endif