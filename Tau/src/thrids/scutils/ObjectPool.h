﻿#ifndef _ObjectPool_h_
#define _ObjectPool_h_

#include"scutils/Utils.h"

namespace scutils
{
	template <class T>
	class ObjectPool
	{
		using AllocMethodCallBack = T * (*)();

	public:
		ObjectPool(int count = 50)
		{

		}

		~ObjectPool()
		{
			for (int i = 0; i < pos; i++)
			{
				DEL(objList[i]);
			}

			pos = -1;
		}

		AllocMethodCallBack NewMethod = nullptr;

		void CreatePool(int count)
		{
			for (int i = 0; i < count; i++)
			{
				T* obj;
				if (NewMethod != nullptr) obj = NewMethod();
				else obj = new T();
				pos++;
				objList.push_back(nullptr);
				objList[pos] = obj;
			}
		}

		T* Pop()
		{
			locker.lock();
			if (pos < 0)
				CreatePool(createCount);

			T* obj = objList[pos];
			objList[pos] = nullptr;
			pos--;

			locker.unlock();

			return obj;
		}

		void Push(T* obj)
		{
			locker.lock();

			size_t size = objList.size();

			if (pos > 50)
			{
				for (int i = 51; i <= pos; i++)
				{
					DEL(objList[i]);
				}

				objList.resize(size - (pos - 50));
				pos = 50;
			}

			pos++;

			if (pos >= objList.size())
			{
				printf("注意重复存取!\n");
				objList.push_back(obj);
			}
			else
			{
				objList[pos] = obj;
			}


			locker.unlock();
		}

		void SetPerCreateCount(int count)
		{
			createCount = count;
		}

	private:
		vector<T*> objList;
		int pos = -1;
		int createCount = 20;
		mutex locker;
	};
}

#endif
