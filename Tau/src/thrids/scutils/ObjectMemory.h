#ifndef _OBJECTMEMORY_H_
#define _OBJECTMEMORY_H_

#include"Utils.h"

namespace scutils
{
	//objectÄÚ´æ
	class DLL_CLASS ObjectMemory
	{
	public:

		ObjectMemory() {}
		virtual ~ObjectMemory() {}


		void* operator new(size_t size)
		{
			return MALLOC(size);
		}

		void operator delete(void* p)
		{
			FREE(p);
		}


		void* operator new[](size_t size)
		{
			return MALLOC(size);
		}

			void operator delete[](void* p)
		{
			FREE(p);
		}
	};


	class DLL_CLASS ObjectMemoryRef
	{
	public:

		ObjectMemoryRef()
			:ref(1)
		{
		}
		virtual ~ObjectMemoryRef() {}

		void addRef() { ref++; }
		int32_t getRef() { return ref; }

		void release()
		{
			ref--;
			if (ref == 0) {
				delete this;
			}
		}

		void* operator new(size_t size)
		{
			return MALLOC(size);
		}


		void operator delete(void* p)
		{
			if (--((ObjectMemoryRef*)p)->ref <= 0)
				FREE(p);
		}

		void* operator new[](size_t size) {return 0; }
			void operator delete[](void* p) {}

	private:
		int32_t ref;
	};

}

#endif