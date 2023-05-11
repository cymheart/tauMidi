#include"TauPool.h"

namespace tau
{
	BUILD_SHARE(TauPool)

	TauPool::TauPool()
	{
		keySounderPool.CreatePool(20);
		zoneSounderPool.CreatePool(20);
		renderEventPool.CreatePool(20);
	}

	TauPool::~TauPool()
	{

	}

}
