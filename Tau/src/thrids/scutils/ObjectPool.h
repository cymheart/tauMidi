#ifndef _ObjectPool_h_
#define _ObjectPool_h_

#include"scutils/Utils.h"

namespace scutils
{
	template <class T>
	class ObjectPool
	{
	public:
		ObjectPool(int count = 50)
		{
			CreatePool(count);
		}

		~ObjectPool()
		{
			for (int i = 0; i < pos; i++)
			{
				DEL(objList[i]);
			}

			pos = -1;
		}

		void CreatePool(int count)
		{
			for (int i = 0; i < count; i++)
			{
				T* obj = new T();
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

			/*if (pos > 300)
			{
				for (int i = 101; i <= pos; i++)
				{
					DEL(objList[i]);
				}

				objList.resize(size - (pos - 100));
				pos = 100;
			}*/

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
