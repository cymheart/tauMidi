#ifndef _Semaphore_h_
#define _Semaphore_h_

#include"Utils.h"

namespace scutils
{
	class Semaphore
	{
	public:
		Semaphore() : count(0) {}
		void set();
		void wait();
		void wait_for(uint32_t ms);

	private:
		mutex m;
		condition_variable cv;
		int count;
	};
}

#endif