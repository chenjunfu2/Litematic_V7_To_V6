#include "Litematic_V7_To_V6.h"
#include "util/CodeTimer.hpp"

#include <stdio.h>
#include <stdlib.h>


template <typename T, typename... Args>
requires (std::is_same_v<std::invoke_result_t<T, Args...>, bool>)
bool CallFuncNoThrow(const T &tFunc, Args... args)
{
	try
	{
		return tFunc(std::forward<Args>(args)...);
	}
	catch (const std::exception &e)
	{
		printf("Catch Error: [%s]\n", e.what());
		return false;
	}
	catch (...)
	{
		printf("Catch Unknown Error!\n");
		return false;
	}
}


int main(int argc, char *argv[])
{
	printf("Get [%d] File(s)\n", argc - 1);

	int iSuccess = 0;
	CodeTimer t;
	for (int i = 1; i < argc; ++i)
	{
		printf("\n[%d]: [%s]\n", i, argv[i]);
		t.Start();
		iSuccess += (int)CallFuncNoThrow(ConvertLitematicFile_V7_To_V6, argv[i]);
		t.Stop();
		t.PrintElapsed("Use Time: [", "]\n");
	}

	printf("\nTotal: [%d], Success: [%d], Fail: [%d]\n", argc - 1, iSuccess, argc - 1 - iSuccess);

#ifndef _DEBUG
	system("pause");
#endif // !_DEBUG

	return 0;
}
