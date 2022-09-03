#include"Utils.h"
#include <chrono>

namespace scutils
{
	int ScUtils_GetCPUCount()
	{
#ifdef _WIN32
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwNumberOfProcessors;
#else
		return android_getCpuCount();
#endif
	}

	/// <summary>
	/// 判断当前系统是大端还是小端
	/// </summary>
	/// <param name=""></param>
	/// <returns>//如果是小端则返回1，如果是大端则返回0</returns>
	DLL_FUNC int JudgeLittleOrBigEndianSystem()
	{
		int a = 1;
		//如果是小端则返回1，如果是大端则返回0
		return *(char*)&a;
	}


	DLL_FUNC int64_t GetCurrentTimeMsec()
	{
		return std::chrono::duration_cast<chrono::milliseconds>(
			chrono::high_resolution_clock::now().time_since_epoch()).count();

		//
//#ifdef _WIN32
//		struct timeval tv;
//		time_t clock;
//		struct tm tm;
//		SYSTEMTIME wtm;
//
//		GetLocalTime(&wtm);
//		tm.tm_year = wtm.wYear - 1900;
//		tm.tm_mon = wtm.wMonth - 1;
//		tm.tm_mday = wtm.wDay;
//		tm.tm_hour = wtm.wHour;
//		tm.tm_min = wtm.wMinute;
//		tm.tm_sec = wtm.wSecond;
//		tm.tm_isdst = -1;
//		clock = mktime(&tm);
//		tv.tv_sec = clock;
//		tv.tv_usec = wtm.wMilliseconds * 1000;
//		return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
//#else
//		struct timeval tv;
//		gettimeofday(&tv, NULL);
//		return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
//#endif
	}

	DLL_FUNC vector<string> split(const string& s, const string& seperator)
	{
		vector<string> result;
		typedef string::size_type string_size;
		string_size i = 0;

		while (i != s.size()) {
			//找到字符串中首个不等于分隔符的字母；
			int flag = 0;
			while (i != s.size() && flag == 0) {
				flag = 1;
				for (string_size x = 0; x < seperator.size(); ++x)
					if (s[i] == seperator[x]) {
						++i;
						flag = 0;
						break;
					}
			}

			//找到又一个分隔符，将两个分隔符之间的字符串取出；
			flag = 0;
			string_size j = i;
			while (j != s.size() && flag == 0) {
				for (string_size x = 0; x < seperator.size(); ++x)
					if (s[j] == seperator[x]) {
						flag = 1;
						break;
					}
				if (flag == 0)
					++j;
			}
			if (i != j) {
				result.push_back(s.substr(i, j - i));
				i = j;
			}
		}
		return result;
	}

}
