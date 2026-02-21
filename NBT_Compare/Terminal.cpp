#include <Windows.h>

//启用虚拟终端序列
bool EnableVirtualTerminalProcessing(void) noexcept
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
	{
		return false;
	}

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;//启用虚拟终端序列
	//dwMode |= DISABLE_NEWLINE_AUTO_RETURN;//关闭自动换行

	return SetConsoleMode(hOut, dwMode);
}