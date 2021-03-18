#include"UniqueID.h"

namespace scutils
{
#define   sequenceMask  (-1L ^ (-1L << 12L))  
#define EPOCHFILETIME 11644473600000000Ui64

	BUILD_SHARE(UniqueID)
		UniqueID::UniqueID()
		:workid(0)
		, seqid(0)
		, last_stamp(0)
	{

	}

	UniqueID::~UniqueID()
	{

	}

	void UniqueID::set_workid(int workid)
	{
		this->workid = workid;
	}


	uint64_t UniqueID::get_curr_ms()
	{
		return GetCurrentTimeMsec();
	}

	uint64_t UniqueID::wait_next_ms(uint64_t lastStamp)
	{
		uint64_t cur = 0;
		do {
			cur = get_curr_ms();
		} while (cur <= lastStamp);
		return cur;
	}

	uint64_t UniqueID::gen()
	{
		uint64_t  uniqueId = 0;
		uint64_t nowtime = get_curr_ms();
		uniqueId = nowtime << 22;
		uniqueId |= (uint64_t)(workid & 0x3ff) << 12;

		if (nowtime < last_stamp)
		{
			perror("error");
			exit(-1);
		}
		if (nowtime == last_stamp)
		{
			seqid = (++seqid) & sequenceMask;

			if (seqid == 0)
			{
				nowtime = wait_next_ms(last_stamp);
				uniqueId = nowtime << 22;
				uniqueId |= (uint64_t)(workid & 0x3ff) << 12;
			}
		}
		else
		{
			seqid = 0;
		}
		last_stamp = nowtime;
		uniqueId |= seqid;
		return uniqueId;
	}


	uint64_t UniqueID::gen_multi()
	{
		uint64_t  uniqueId = 0;
		threadLock.lock();
		uniqueId = gen();
		threadLock.unlock();
		return uniqueId;
	}
}
