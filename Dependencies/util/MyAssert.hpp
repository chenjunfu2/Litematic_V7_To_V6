#pragma once

#include <stdio.h>//printf
#include <stdlib.h>//exit
#include <stdarg.h>//va_arg

//必须没有提前定义过任何此类宏
#if !defined(COMPILER_MSVC) &&\
	!defined(COMPILER_GCC) &&\
	!defined(COMPILER_CLANG)

	//先预定义所有可能的编译器宏
	#define COMPILER_MSVC 0
	#define COMPILER_GCC 0
	#define COMPILER_CLANG 0
	
	//后实际判断是哪个编译器，是就替换它自己的宏为1
	#if defined(_MSC_VER)
		#undef  COMPILER_MSVC
		#define COMPILER_MSVC 1
		#define COMPILER_NAME "MSVC"
	#elif defined(__GNUC__)
		#undef  COMPILER_GCC
		#define COMPILER_GCC 1
		#define COMPILER_NAME "GCC"
	#elif defined(__clang__)
		#undef  COMPILER_CLANG
		#define COMPILER_CLANG 1
		#define COMPILER_NAME "Clang"
	#else
		#define COMPILER_NAME "Unknown"
	#endif
#endif


#if COMPILER_MSVC
#define PRINTF_FORMAT_ARGS _Printf_format_string_
#define PRINTF_FORMAT_ATTR
#elif COMPILER_GCC || COMPILER_CLANG
#define PRINTF_FORMAT_ARGS
#define PRINTF_FORMAT_ATTR __attribute__((__format__ (__printf__, 4, 5)))
#else
#define PRINTF_FORMAT_ARGS
#define PRINTF_FORMAT_ATTR
#endif


PRINTF_FORMAT_ATTR
inline void MyAssert_Function(const char *pFileName, size_t szLine, const char *pFunctionName, PRINTF_FORMAT_ARGS const char *pInfo = NULL, ...)
{
	printf("Assertion Failure!\n    in file: %s\n    in line: %zu\n    in func: %s\n    in info: ", pFileName, szLine, pFunctionName);
	
	if (pInfo != NULL)
	{
		va_list vl;
		va_start(vl, pInfo);
		vprintf(pInfo, vl);
		va_end(vl);
	}
	else
	{
		printf("[No Info]");
	}
	
	//帮忙换行
	putchar('\n');

	exit(-1);
}

//代理宏，延迟展开
#define MY_ASSERT_FILE __FILE__
#define MY_ASSERT_LINE __LINE__
#define MY_ASSERT_FUNC __FUNCTION__

//cpp20的__VA_OPT__(,)，仅在__VA_ARGS__不为空时添加','以防止编译错误
//msvc需启用"/Zc:preprocessor"以使得预处理器识别此宏关键字（哎呀msvc你怎么这么坏呀）
#define MyAssert(v, ...)\
do\
{\
	if(!(v))\
	{\
		MyAssert_Function(MY_ASSERT_FILE, MY_ASSERT_LINE, MY_ASSERT_FUNC __VA_OPT__(, ) __VA_ARGS__);\
	}\
}while(0)


#undef PRINTF_FORMAT_ATTR
#undef PRINTF_FORMAT_ARGS

#undef COMPILER_NAME
#undef COMPILER_CLANG
#undef COMPILER_GCC
#undef COMPILER_MSVC