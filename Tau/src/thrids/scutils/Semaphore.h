#ifndef _Semaphore_h_
#define _Semaphore_h_

#include"Utils.h"

namespace scutils
{
	class Semaphore
	{
	public:
		Semaphore(int value = 0) : count(value), initCount(value) {}
		void reset(int value) { initCount = count = value; }
		void set();
		void wait();
		void wait_for(uint32_t ms);

	private:
		mutex m;
		condition_variable cv;
		int count;
		int initCount;
	};
}

#endif
