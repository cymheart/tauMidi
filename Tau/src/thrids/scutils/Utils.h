﻿#ifndef _Utils_h_
#define _Utils_h_

#ifdef _WIN32
#include <Windows.h>
#include <MMSystem.h>
#else
#include <cpu-features.h>
#endif

#include <vector>
#include <string>
#include <unordered_map>  
#include <unordered_set>  
#include <functional>
#include <atomic>
#include <math.h>
#include <cstdint>
#include <list>

using namespace std;

#if defined(SC_DEBUG)
#   if defined(_MSC_VER)
#       // "(void)0," is for suppressing C4127 warning in "assert(false)", "assert(true)" and the like
#       define SCASSERT( x )           if ( !((void)0,(x))) { __debugbreak(); }
#   elif defined (ANDROID_NDK)
#       include <android/log.h>
#       define SCASSERT( x )           if ( !(x)) { __android_log_assert( "assert", "grinliz", "ASSERT in '%s' at %d.", __FILE__, __LINE__ ); }
#   else
#       include <assert.h>
#       define TIXMLASSERT                assert
#   endif
#else
#   define SCASSERT( x )               {}
#endif

#ifdef _WIN32
#  ifdef TAU_EXPORT
#     define DLL_CLASS __declspec(dllexport)
#     define DLL_FUNC __declspec(dllexport)
#  elif defined(TAU_IMPORT)
#     define DLL_CLASS __declspec(dllimport)
#     define DLL_FUNC __declspec(dllimport)
#  else
#     define DLL_CLASS
#     define DLL_FUNC
#  endif
#else
#     define DLL_CLASS
#     define DLL_FUNC
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846264338327950288   /**< pi */
#endif

#define MAX_DELAY_TIME  0x7FFFFFFF
#define Delegate(func) typedef function<func>

#define MALLOC(size)                   malloc(size);
#define CALLOC(num, size)              calloc(num, size);
#define FREE(ptr)                      {if(ptr != nullptr){ free(ptr); ptr = nullptr;}}


// 释放指针宏
#define DEL(obj)                      {if(obj != nullptr ){delete obj;obj=nullptr;}}

// 释放数组指针宏
#define DEL_ARRAY(objs)                {if(objs != nullptr ){delete [] objs;objs=nullptr;}}

//释放指针vector中的指针obj
#define DEL_OBJS_VECTOR(objVec) \
{\
if (objVec != nullptr)\
{\
    for (int i = 0; i < objVec->size(); i++)\
        DEL((*objVec)[i]);\
    DEL(objVec);\
}}

//释放指针list中的指针obj
#define DEL_OBJS_LIST(objList, ojbListType) \
{\
if(objList != nullptr)\
{\
    ojbListType::iterator it = objList->begin(); \
    ojbListType::iterator end = objList->end();  \
        for (; it != end; it++)\
            DEL(*it); \
        DEL(objList); \
}}

#define SWAP(T, a, b)  do{T tmp = a; a= b; b = tmp;}while(0)

#ifndef _WIN32
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif

namespace scutils
{
	DLL_FUNC int JudgeLittleOrBigEndianSystem();

	DLL_FUNC vector<string> split(const string& s, const string& seperator);
	DLL_FUNC bool contains(const string& str, const string& substr);
	DLL_FUNC int64_t GetCurrentTimeMsec();

	int ScUtils_GetCPUCount();

	template <typename T>
	class Comparator
	{
	public:
		virtual int compare(T left, T right) = 0;
	};

	template <typename T> // T must be integer type
	T to_little_endian(T x)
	{
		size_t n = sizeof(T) / sizeof(uint8_t); // 2,4,8

		T res;
		uint8_t* p = (uint8_t*)&res;
		T mask = static_cast<T>(0xFF);

		for (size_t i = 0; i < n; i++)
		{
			size_t offset = 8 * i;
			p[i] = (x & (mask << offset)) >> offset;
		}

		return res;
	}

	template <typename T> // T must be integer type
	T to_big_endian(T x)
	{
		size_t n = sizeof(T) / sizeof(uint8_t); // 2,4,8

		T res;
		uint8_t* p = (uint8_t*)&res;
		T mask = static_cast<T>(0xFF);

		for (size_t i = 0; i < n; i++)
		{
			int offset = (int)(8 * (n - i - 1));
			p[i] = (x & (mask << offset)) >> offset;
		}

		return res;
	}

	
	char* FormatStr(const char* format, ...);


}

#endif
