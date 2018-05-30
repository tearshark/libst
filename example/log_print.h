#pragma once
#include <string>

#ifdef _WIN32
#include <windows.h>
inline void __log_print_impl(std::string & str)
{
	str += "\r\n";
	::OutputDebugStringA(str.c_str());
}
#elif __APPLE__ 
inline void __log_print_impl(std::string & str)
{
	printf("%s\n", str.c_str());
}
#elif __ANDROID__
#include <android/log.h>
inline void __log_print_impl(std::string & str)
{
	__android_log_write(ANDROID_LOG_INFO, "libtask", str.c_str());
}
#else
#include <iostream>
inline void __log_print_impl(std::string & str)
{
	std::cout << str << std::endl;
}
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include <sstream>

namespace lib_shark_task
{
	namespace detail
	{
		template<class _Ty>
		inline std::string to_string(_Ty&& _value)
		{
			return std::string{};
		}
		inline std::string to_string(const std::string & _value)
		{
			return _value;
		}
		inline std::string to_string(std::string & _value)
		{
			return _value;
		}
		inline std::string to_string(std::string && _value)
		{
			return std::forward<std::string>(_value);
		}

		inline std::string to_string(void * _value)
		{
			char szbuf[128];
			sprintf(szbuf, "0x%p", _value);
			return std::string(szbuf);
		}

		inline std::string to_string(int _value)
		{
			char szbuf[128];
			sprintf(szbuf, "%d", _value);
			return std::string(szbuf);
		}
		inline std::string to_string(long _value)
		{
			char szbuf[128];
			sprintf(szbuf, "%ld", _value);
			return std::string(szbuf);
		}
		inline std::string to_string(long long _value)
		{
			char szbuf[128];
			sprintf(szbuf, "%lld", _value);
			return std::string(szbuf);
		}
		inline std::string to_string(unsigned int _value)
		{
			char szbuf[128];
			sprintf(szbuf, "%u", _value);
			return std::string(szbuf);
		}
		inline std::string to_string(unsigned long _value)
		{
			char szbuf[128];
			sprintf(szbuf, "%lu", _value);
			return std::string(szbuf);
		}
		inline std::string to_string(unsigned long long _value)
		{
			char szbuf[128];
			sprintf(szbuf, "%llu", _value);
			return std::string(szbuf);
		}
		inline std::string to_string(const char * _value)
		{
			return std::string(_value);
		}
		inline std::string to_string(std::thread::id tid)
		{
			std::stringstream ss;
			ss << tid;
			return ss.str();
		}


		inline void log_concat(std::string & str)
		{
		}
		template<class _Ty, class... _Rest>
		inline void log_concat(std::string & str, _Ty&& val, _Rest&&... rest)
		{
			str += to_string(val);
			log_concat(str, std::forward<_Rest>(rest)...);
		}
	}
}

#ifdef _MSC_VER
#pragma warning(default : 4996)
#endif

template<class... _Types>
inline void log_print(_Types&&... args)
{
	std::string str;
	lib_shark_task::detail::log_concat(str, std::forward<_Types>(args)...);
	__log_print_impl(str);
}
