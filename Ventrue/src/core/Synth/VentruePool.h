#ifndef _VentruePool_h_
#define _VentruePool_h_

#include "VentrueTypes.h"
#include"VentrueEvent.h"
#include"KeySounder.h"
#include"RegionSounder.h"


namespace ventrue
{
	class VentruePool
	{
		SINGLETON(VentruePool)

	public:

		ObjectPool<KeySounder>& KeySounderPool() { return keySounderPool; }
		ObjectPool<RegionSounder>& RegionSounderPool() { return regionSounderPool; }
		ObjectPool<VentrueEvent>& VentrueEventPool() { return ventrueEventPool; }

	private:
		ObjectPool<KeySounder> keySounderPool;
		ObjectPool<RegionSounder> regionSounderPool;
		ObjectPool<VentrueEvent> ventrueEventPool;
	};

}
#endif
