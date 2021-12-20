#include"MsgProcesser.h"

namespace task
{

	void MsgProcesser::Init(TaskProcesser* taskProcesser)
	{
		this->taskProcesser = taskProcesser;
	}

	/**注册需要监听的消息任务 */
	int MsgProcesser::RegMsg(int32_t msg, MsgCallBack msgCB)
	{
		if (msgCB != nullptr)
		{
			vector<MsgCallBack>& vec = msgTaskInfoMap[msg];
			vector<MsgCallBack>::iterator it = find(vec.begin(), vec.end(), msgCB);
			if (it != vec.end())
				return 0;

			vec.push_back(msgCB);
			return 0;
		}

		return 1;
	}

	/**取消注册消息任务处理函数 */
	void MsgProcesser::UnRegMsgFunc(int32_t msg, MsgCallBack msgCB)
	{
		if (msgCB == nullptr)
		{
			UnRegMsg(msg);
			return;
		}

		auto it = msgTaskInfoMap.find(msg);
		if (it == msgTaskInfoMap.end())
			return;

		vector<MsgCallBack>& vec = it->second;

		vector<MsgCallBack>::iterator vit = find(vec.begin(), vec.end(), msgCB);
		if (vit == vec.end())
			return;
		vec.erase(vit);

		if (vec.empty())
			msgTaskInfoMap.erase(msg);

	}

	/**取消注册指定消息任务 */
	void MsgProcesser::UnRegMsg(int32_t msg)
	{
		msgTaskInfoMap.erase(msg);
	}

	/**取消注册所有消息任务 */
	void MsgProcesser::UnRegAllMsg()
	{
		msgTaskInfoMap.clear();
	}

	/**发送消息任务 */
	void MsgProcesser::SendMsg(int32_t msg, void* data, int delayMS)
	{
		ProcessMsg(msg, data, delayMS);
	}

	/**发送消息任务 */
	void  MsgProcesser::SendMsg(int32_t msg) {
		SendMsg(msg, nullptr, 0);
	}

	/**发送消息任务 */
	void MsgProcesser::SendMsg(int32_t msg, int delayMS)
	{
		SendMsg(msg, nullptr, delayMS);
	}

	void MsgProcesser::ProcessMsg(int32_t msg, void* data, int delayMS)
	{
		auto it = msgTaskInfoMap.find(msg);
		if (it == msgTaskInfoMap.end())
			return;

		vector<MsgCallBack>& msgTaskInfoList = it->second;
		Task* task = taskProcesser->PopTask();
		task->msg = TaskMsg::TMSG_DATA;
		task->processCallBack = MsgTask;
		task->data = &(msgTaskInfoList);
		task->exData[0] = (void*)msg;
		task->exData[1] = data;
		taskProcesser->PostTask(task, delayMS);
	}

	void MsgProcesser::MsgTask(Task* task)
	{
		vector<MsgCallBack>& msgTaskInfoList = *(vector<MsgCallBack>*)(task->data);
		uintptr_t msg = (uintptr_t)task->exData[0];
		for (int i = 0; i < msgTaskInfoList.size(); i++)
		{
			msgTaskInfoList[i](msg, task->exData[1]);
		}
	}
}
