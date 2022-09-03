#ifndef REVERB_H
#define REVERB_H

#include<stdlib.h>


namespace oteypiano
{

	class Reverb {
	public:
		Reverb(float c1, float c3, float a, float mix, float Fs);
		~Reverb();
		float tick(float in);
		void* operator new(size_t sz) {
			return malloc(sizeof(Reverb));
		}
		void operator delete(void* pObject) {
			free(pObject);
		}
		float mix;
		Delay d[8];
		float A[8][8];
		float o[8];
		float b[8];
		float c[8];
		Filter decay[8];
	};

}

#endif
