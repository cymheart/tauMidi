#include"VentrueEvent.h"

namespace ventrue
{
	void VentrueEvent::Release(Task* task)
	{
		VentruePool::GetInstance().VentrueEventPool().Push((VentrueEvent*)task);
	}

}
