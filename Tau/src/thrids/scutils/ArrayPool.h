#ifndef _ArrayPool_h_
#define _ArrayPool_h_

#include"scutils/Utils.h"

namespace scutils
{
	template <class T>
	class ArrayPool
	{
	public:
		ArrayPool(int count = 10, int arraySize = 1024)
		{
			SetPerCreateCount(count, arraySize);
			CreatePool(count, arraySize);
		}

		~ArrayPool()
		{
			for (int i = 0; i < pos; i++)
			{
				DEL_ARRAY(objList[i]);
			}

			pos = -1;
		}

		void CreatePool(int count, int arraySize)
		{
			for (int i = 0; i < count; i++)
			{
				T* objs = new T[arraySize];
				pos++;
				objList.push_back(nullptr);
				objList[pos] = objs;
			}
		}

		T* Pop()
		{
			if (pos < 0)
				CreatePool(createCount, arraySize);

			T* obj = objList[pos];
			objList[pos] = nullptr;
			pos--;

			return obj;
		}

		void Push(T* obj)
		{
			size_t size = objList.size();
			pos++;

			if (pos >= objList.size())
			{
				printf("◊¢“‚÷ÿ∏¥¥Ê»°!\n");
				objList.push_back(obj);
			}
			else
			{
				objList[pos] = obj;
			}
		}

		void SetPerCreateCount(int count, int _arraySize)
		{
			createCount = count;
			arraySize = _arraySize;
		}

	private:
		vector<T*> objList;
		int pos = -1;
		int createCount = 10;
		int arraySize = 1024;
	};
}

#endif
