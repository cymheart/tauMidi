#ifndef _TauPool_h_
#define _TauPool_h_

#include "TauTypes.h"
#include"SyntherEvent.h"
#include"KeySounder.h"
#include"ZoneSounder.h"


namespace tau
{
	class TauPool
	{
		SINGLETON(TauPool)

	public:

		ObjectPool<KeySounder>& KeySounderPool() { return keySounderPool; }
		ObjectPool<ZoneSounder>& ZoneSounderPool() { return zoneSounderPool; }
		ObjectPool<SyntherEvent>& RenderEventPool() { return renderEventPool; }

	private:
		ObjectPool<KeySounder> keySounderPool;
		ObjectPool<ZoneSounder> zoneSounderPool;
		ObjectPool<SyntherEvent> renderEventPool;
	};

}
#endif
