#include "Litematic_V7_To_V6.h"
#include "util/CodeTimer.hpp"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	printf("Get [%d] File(s)\n", argc - 1);

	int iSuccess = 0;
	CodeTimer t;
	for (int i = 1; i < argc; ++i)
	{
		printf("\n[%d]: [%s]\n", i, argv[i]);
		t.Start();
		iSuccess += (int)ConvertLitematicFile_V7_To_V6(argv[i]);
		t.Stop();
		t.PrintElapsed("Use Time: [", "]\n");
	}

	printf("\nTotal: [%d], Success: [%d], Fail: [%d]\n", argc - 1, iSuccess, argc - 1 - iSuccess);

#ifndef _DEBUG
	system("pause");
#endif // !_DEBUG

	return 0;
}
