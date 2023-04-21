#include"Semaphore.h"

namespace scutils {
	void Semaphore::set() {
		unique_lock <mutex> lock(m);
		--count;
		cv.notify_one();
	}

	void Semaphore::wait() {
		unique_lock <mutex> lock(m);
		while (count >= 0) {
			cv.wait(lock);
		}
		count = initCount;
	}

	void Semaphore::wait_for(uint32_t ms) {
		unique_lock <mutex> lock(m);
		if (count >= 0) {
			cv_status s = cv.wait_for(lock, chrono::milliseconds(ms));
		}
		count = initCount;
	}
}
