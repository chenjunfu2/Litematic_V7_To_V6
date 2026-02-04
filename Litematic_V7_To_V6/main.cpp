#include "Litematic_V7_To_V6.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("Get [%d] File(s)\n", argc - 1);

	int iSuccess = 0;
	for (int i = 1; i < argc; ++i)
	{
		printf("\n[%d]: [%s]\n", i, argv[i]);
		iSuccess += (int)ConvertLitematicFile_V7_To_V6(argv[i]);
	}

	printf("\nTotal: [%d], Success: [%d], Fail: [%d]\n", argc - 1, iSuccess, argc - 1 - iSuccess);

	return 0;
}
