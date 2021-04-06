#include"VentrueEvent.h"

namespace ventrue
{
	VentrueEvent* VentrueEvent::New()
	{
		VentrueEvent* ev = VentruePool::GetInstance().VentrueEventPool().Pop();
		//VentrueEvent* ev = new VentrueEvent();
		return ev;
	}

	void VentrueEvent::Release(Task* task)
	{
		VentruePool::GetInstance().VentrueEventPool().Push((VentrueEvent*)task);
	}

}
