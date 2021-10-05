#include"SyntherEvent.h"

namespace tau
{
	SyntherEvent* SyntherEvent::New()
	{
		SyntherEvent* ev = TauPool::GetInstance().RenderEventPool().Pop();
		ev->msg = TaskMsg::TMSG_DATA;
		//SyntherEvent* ev = new SyntherEvent();
		return ev;
	}

	void SyntherEvent::Release(Task* task)
	{
		task->msg = TaskMsg::TMSG_DATA;
		TauPool::GetInstance().RenderEventPool().Push((SyntherEvent*)task);
	}

}
