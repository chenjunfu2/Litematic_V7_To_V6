#include "nbt_cpp/NBT_All.hpp"
#include "util/CodeTimer.hpp"
#include "util/MyAssert.hpp"

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <thread>

//找到一个唯一文件名
std::string GenerateUniqueFilename(const std::string &sBeg, const std::string &sEnd, uint32_t u32TryCount = 10)//默认最多重试10次
{
	while (u32TryCount != 0)
	{
		//时间用[]包围
		auto tmpPath = std::format("{}[{}]{}", sBeg, CodeTimer::GetSystemTime(), sEnd);//获取当前系统时间戳作为中间的部分
		if (!NBT_IO::IsFileExist(tmpPath))
		{
			return tmpPath;
		}

		//等几ms在继续
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		--u32TryCount;
	}

	//次数到上限直接返回空
	return std::string{};
}


bool ConvertLitematicData_V7_To_V6(const NBT_Type::Compound &cpdV7Input, NBT_Type::Compound &cpdV6Output)
{








	return true;
}


bool ConvertLitematicFile_V7_To_V6(const std::string &sV7FilePath)
{
	std::vector<uint8_t> u8FileStream{};
	if (!NBT_IO::ReadFile(sV7FilePath, u8FileStream))
	{
		printf("Unable to read stream from file!\n");
		return false;
	}

	NBT_Type::Compound cpdV7Input{};
	if (!NBT_Reader::ReadNBT(u8FileStream, cpdV7Input))
	{
		printf("Unable to parse data from stream!\n");
		return false;
	}

	//使用后清除u8FileStream
	u8FileStream.clear();

	NBT_Type::Compound cpdV6Output{};
	if (!ConvertLitematicData_V7_To_V6(cpdV7Input, cpdV6Output))
	{
		printf("Unable to convert v7_data to v6_data!\n");
		return false;
	}

	//重用u8FileStream
	if (!NBT_Writer::WriteNBT(u8FileStream, cpdV6Output))
	{
		printf("Unable to write data into stream!\n");
		return false;
	}

	//查找合法文件
	std::string sV6FilePath{};
	{
		//找到后缀名
		size_t szPos = sV7FilePath.find_last_of('.');

		//'.'前面的部分，不包含'.'
		std::string sV7FileName = sV7FilePath.substr(0, szPos).append("_V6_");
		//'.'后面的部分，包含'.'
		std::string sV7FileExten = sV7FilePath.substr(szPos);

		//唯一文件名
		sV6FilePath = GenerateUniqueFilename(sV7FileName, sV7FileExten);
		if (sV6FilePath.empty())
		{
			printf("Unable to find a valid file name or lack of permission!\n");
			return false;
		}
	}

	//写入数据
	if (!NBT_IO::WriteFile(sV6FilePath, u8FileStream))
	{
		printf("Unable to write stream into file!\n");
		return false;
	}

	return true;
}


