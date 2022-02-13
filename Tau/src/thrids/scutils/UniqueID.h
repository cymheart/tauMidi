#ifndef _UniqueID_h_
#define _UniqueID_h_

#include"Utils.h"
#include <time.h>
#include"SingletonDefine.h"
#include<Mutex>


/*
ID 生成策略
毫秒级时间41位+机器ID 10位+毫秒内序列12位。
0 41 51 64 +-----------+------+------+ |time |pc |inc | +-----------+------+------+
前41bits是以毫秒为单位的timestamp。
接着10bits是事先配置好的机器ID。
最后12bits是累加计数器。
macheine id(10bits)标明最多只能有1024台机器同时产生ID，sequence number(12bits)也标明1台机器1ms中最多产生4096个ID， *
*/
namespace scutils
{
	class UniqueID
	{
		SINGLETON(UniqueID)

	public:
		void set_workid(int workid);
		uint64_t gen();
		uint64_t gen_multi();

	private:
		uint64_t get_curr_ms();
		uint64_t wait_next_ms(uint64_t lastStamp);

	private:
		mutex threadLock;
		uint64_t last_stamp;
		int workid;
		uint64_t seqid;
	};
}

#endif
