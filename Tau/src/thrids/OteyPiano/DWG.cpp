
#include "DWG.h"

namespace oteypiano
{

	void kill_denormals(float& val)
	{
		static const float anti_denormal = 1e-18;
		val += anti_denormal;
		val -= anti_denormal;
	}

	void kill_denormals(double& val)
	{
		static const double anti_denormal = 1e-18;
		val += anti_denormal;
		val -= anti_denormal;
	}

	float ValimakiDispersion(float B, float f, int M)
	{
		float C1, C2, k1, k2, k3;
		if (M == 4) {
			C1 = .069618;
			C2 = 2.0427;
			k1 = -.00050469;
			k2 = -.0064264;
			k3 = -2.8743;
		}
		else {
			C1 = .071089;
			C2 = 2.1074;
			k1 = -.0026580;
			k2 = -.014811;
			k3 = -2.9018;
		}

		float logB = log(B);
		float kd = exp(k1 * logB * logB + k2 * logB + k3);
		float Cd = exp(C1 * logB + C2);
		float halfstep = pow(2.0, 1.0 / 12.0);
		float Ikey = log(f * halfstep / 27.5) / log(halfstep);
		float D = exp(Cd - Ikey * kd);
		return D;
	}

	////////groupdelay
	void evalpolyB(float omega, float* h, int N, float H[2]) {
		H[0] = 0; H[1] = 0;
		for (int i = 0; i < N; i++) {
			H[0] += cos(float(i) * omega) * h[i];
			H[1] -= sin(float(i) * omega) * h[i];
		}
	}

	void evalpolyA(float omega, float* h, int N, float H[2]) {
		H[0] = 1; H[1] = 0;
		for (int i = 0; i < N; i++) {
			H[0] += cos(float(i + 1) * omega) * h[i];
			H[1] -= sin(float(i + 1) * omega) * h[i];
		}
	}

	void evalrampedB(float omega, float* h, int N, float H[2]) {
		H[0] = 0; H[1] = 0;
		for (int i = 1; i < N; i++) {
			H[0] += float(i) * cos(float(i) * omega) * h[i];
			H[1] -= float(i) * sin(float(i) * omega) * h[i];
		}
	}

	void evalrampedA(float omega, float* h, int N, float H[2]) {
		H[0] = 0; H[1] = 0;
		for (int i = 0; i < N; i++) {
			float j = float(i) + 1.0;
			H[0] += j * cos(j * omega) * h[i];
			H[1] -= j * sin(j * omega) * h[i];
		}
	}

	void complex_divide(float A[2], float B[2], float C[2]) {
		float magd2 = B[0] * B[0] + B[1] * B[1];
		C[0] = (A[0] * B[0] + A[1] * B[1]) / magd2;
		C[1] = (A[1] * B[0] - A[0] * B[1]) / magd2;
	}

	void complex_multiply(float A[2], float B[2], float C[2]) {
		C[0] = A[0] * B[0] - A[1] * B[1];
		C[1] = A[0] * B[1] + A[1] * B[0];
	}

	float groupdelay(float f, float* B, int sizeB, float* A, int sizeA, float FS) {
		float omega = 2.0 * M_PI * f / FS;
		float Br[2]; evalrampedB(omega, B, sizeB, Br);
		float Be[2]; evalpolyB(omega, B, sizeB, Be);
		float Ar[2]; evalrampedA(omega, A, sizeA, Ar);
		float Ae[2]; evalpolyA(omega, A, sizeA, Ae);
		float BrxAe[2]; complex_multiply(Br, Ae, BrxAe);
		float BexAr[2]; complex_multiply(Be, Ar, BexAr);
		float AxB[2]; complex_multiply(Ae, Be, AxB);
		float Num[2]; Num[0] = BrxAe[0] - BexAr[0]; Num[1] = BrxAe[1] - BexAr[1];
		float c[2]; complex_divide(Num, AxB, c);
		return c[0];
	}
	////
	void evalpoly(float omega, float* B, int sizeB, float* A, int sizeA, float H[2]) {
		float HB[2];
		float HA[2];
		evalpolyB(omega, B, sizeB, HB);
		evalpolyA(omega, A, sizeA, HA);
		complex_divide(HB, HA, H);
	}
	float PhaseIIR(float omega, float* B, int sizeB, float* A, int sizeA) {
		float C[2];
		evalpoly(omega, B, sizeB, A, sizeA, C);
		return atan2(C[1], C[0]);
	}
	float PhaseDelayDerive(float omega, float* B, int sizeB, float* A, int sizeA, float delta) {
		float omega1 = omega - delta;
		float omega2 = omega + delta;
		float p1 = PhaseIIR(omega1, B, sizeB, A, sizeA);
		float p2 = PhaseIIR(omega2, B, sizeB, A, sizeA);
		return (-omega1 * p2 + omega2 * p1) / (2 * delta * omega1 * omega2);
	}

	float PhaseDelay(float f, float* B, int sizeB, float* A, int sizeA, float FS) {
		float grpdel = groupdelay(f, B, sizeB, A, sizeA, FS);
		float omega = 2.0 * M_PI * f / FS;
		return grpdel - omega * PhaseDelayDerive(omega, B, sizeB, A, sizeA);
	}
	///
	long Nchoose(long n, long k)
	{
		long divisor = 1;
		long multiplier = n;
		long answer = 1;
		k = std::min(k, n - k);
		while (divisor <= k)
		{
			answer = (answer * multiplier) / divisor;
			multiplier--;
			divisor++;
		}
		return answer;
	}

	///////////////////////////
	//Circular Buffer

	CircularBufferBase::CircularBufferBase(int size)
	{
		pointer = 0;
		this->size = size;
		Buffer = (float*)malloc(sizeof(float) * size);
		memset(Buffer, 0, size * sizeof(float));

	}

	CircularBufferBase::~CircularBufferBase()
	{
		free(Buffer);
	}

	void CircularBufferBase::push(float a) {
		pointer--;
		if (pointer < 0)
			pointer = size - 1;
		Buffer[pointer] = a;
	}

	void CircularBufferBase::push(float a, int pos) {
		Buffer[pointerInRange(pointer + pos)] = a;
	}

	void CircularBufferBase::add(float a, int pos) {
		int posr = pointerInRange(pointer + pos);
		Buffer[posr] += a;
	}
	void CircularBufferBase::set(float a, int pos) {
		int posr = pointerInRange(pointer + pos);
		Buffer[posr] = a;
	}

	float CircularBufferBase::get()
	{
		return Buffer[pointer];
	}
	float CircularBufferBase::get(int pos)
	{
		return Buffer[pointerInRange(pointer + pos)];
	}

	int CircularBuffer::pointerInRange(int p) {
		p = p % size;
		if (p < 0) { p += size; }
		return p;
	}
	void CircularBuffer::allocate(int size) {
		Buffer = (float*)malloc(sizeof(float) * size);
	}
	float CircularBuffer::delay(int pos)
	{
		assertv(size >= pos);
		int pAl2 = (this->pointer + pos);
		pAl2 = pAl2 % size;
		if (pAl2 < 0) { pAl2 += size; }
		return this->Buffer[pAl2];
	}
}


