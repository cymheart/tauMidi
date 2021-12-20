#ifndef _MsgProcesser_h_
#define _MsgProcesser_h_

#include"TaskProcesser.h"

namespace task
{
	class MsgProcesser
	{
	public:
		void Init(TaskProcesser* taskProcesser);

		/**注册需要监听的消息任务 */
		int RegMsg(int32_t msg, MsgCallBack msgCB);

		/**取消注册消息任务处理函数 */
		void UnRegMsgFunc(int32_t msg, MsgCallBack msgCB);

		/**取消注册指定消息任务 */
		void UnRegMsg(int32_t msg);

		/**取消注册所有消息任务 */
		void UnRegAllMsg();

		/**发送消息任务 */
		void SendMsg(int32_t msg, void* data, int delayMS);

		/**发送消息任务 */
		void SendMsg(int32_t msg);

		/**发送消息任务 */
		void SendMsg(int32_t msg, int delayMS);

	private:
		void ProcessMsg(int32_t msg, void* data, int delayMS);
		static void MsgTask(Task* task);
	private:
		TaskProcesser* taskProcesser;
		unordered_map<int32_t, vector<MsgCallBack>> msgTaskInfoMap;
	};
}

#endif
