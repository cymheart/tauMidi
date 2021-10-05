#ifndef _TauPool_h_
#define _TauPool_h_

#include "TauTypes.h"
#include"SyntherEvent.h"
#include"KeySounder.h"
#include"RegionSounder.h"


namespace tau
{
	class TauPool
	{
		SINGLETON(TauPool)

	public:

		ObjectPool<KeySounder>& KeySounderPool() { return keySounderPool; }
		ObjectPool<RegionSounder>& RegionSounderPool() { return regionSounderPool; }
		ObjectPool<SyntherEvent>& RenderEventPool() { return renderEventPool; }

	private:
		ObjectPool<KeySounder> keySounderPool;
		ObjectPool<RegionSounder> regionSounderPool;
		ObjectPool<SyntherEvent> renderEventPool;
	};

}
#endif
