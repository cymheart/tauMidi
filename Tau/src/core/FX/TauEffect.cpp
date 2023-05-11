#include"TauEffect.h"

namespace tauFX
{
	void TauEffect::LockData() {
		if (tau)
			tau->Lock();
	}

	void TauEffect::UnLockData() {
		if (tau)
			tau->UnLock();
	}
}