#include"Semaphore.h"

namespace scutils {
	void Semaphore::set() {
		//printf("wait_for调用set：\n");
		unique_lock <mutex> lock(m);
		//++count;
		count = 1;
		cv.notify_one();
	}

	void Semaphore::wait() {
		unique_lock <mutex> lock(m);
		while (count == 0)
			cv.wait(lock);
		//--count;
		count = 0;
	}

	void Semaphore::wait_for(uint32_t ms) {
		unique_lock <mutex> lock(m);

		//printf("wait_for的count：%d\n", count);

		while (count == 0) {
			cv_status s = cv.wait_for(lock, chrono::milliseconds(ms));
			count = 0;
			return;
		}

		//--count;
		count = 0;
	}
}
