#include <nbt_cpp/NBT_All.hpp>
#include <util/CodeTimer.hpp>

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

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Use:\n>[%s] [File1] [File2]\nTo Compare\n", argv[0]);
		return 0;
	}

	auto ReadNBT = [](const char *pFileName, NBT_Type::Compound &cpdInput) -> bool
	{
		std::vector<uint8_t> vFileStream1{};
		if (!NBT_IO::ReadFile(pFileName, vFileStream1))
		{
			printf("Unable to read stream from file!\n");
			return false;
		}

		//如果解压失败那么可能原先文件未压缩
		std::vector<uint8_t> vDataV7Stream{};
		if (!NBT_IO::DecompressDataNoThrow(vDataV7Stream, vFileStream1))
		{
			printf("Data may not be compressed, attempt to parse directly.\n");
			vDataV7Stream = std::move(vFileStream1);//尝试以未压缩流处理，而不是失败
		}

		if (!NBT_Reader::ReadNBT(vDataV7Stream, 0, cpdInput))
		{
			printf("Unable to parse data from stream!\n");
			return false;
		}

		return true;
	};

	NBT_Type::Compound cpdInput[2];
	bool b0 = ReadNBT(argv[1], cpdInput[0]);
	bool b1 = ReadNBT(argv[2], cpdInput[1]);

	if (!b0 || !b1)
	{
		printf("ReadNBT fail!\n");
		return 0;
	}

	if (!cpdInput[0].HasCompound(MU8STR("")) ||
		!cpdInput[1].HasCompound(MU8STR("")))
	{
		printf("Root Compound not found!\n");
		return 0;
	}

	auto tmp0 = std::move(cpdInput[0].GetCompound(MU8STR("")));
	cpdInput[0] = std::move(tmp0);

	auto tmp1 = std::move(cpdInput[1].GetCompound(MU8STR("")));
	cpdInput[1] = std::move(tmp1);

	if (cpdInput[0] == cpdInput[1])
	{
		printf("Equal!\n");
		return 0;
	}

	printf("No equal!\n");
	//查找合法文件
	auto FindFileName = [](const std::string &strOldFileName, std::string &strNewFileName) -> bool
	{
		//找到后缀名
		size_t szPos = strOldFileName.find_last_of('.');

		//'.'前面的部分，不包含'.'
		std::string sNewFileName = strOldFileName.substr(0, szPos).append("Cmp");

		//唯一文件名
		strNewFileName = GenerateUniqueFilename(sNewFileName, ".txt");
		if (strNewFileName.empty())
		{
			printf("Unable to find a valid file name or lack of permission!\n");
			return false;
		}

		return true;
	};

	std::string strCmpFileName[2];
	b0 = FindFileName(argv[1], strCmpFileName[0]);
	b1 = FindFileName(argv[2], strCmpFileName[1]);

	if (!b0 || !b1)
	{
		printf("FindFileName fail!\n");
		return 0;
	}


	FILE *pFile[2];
	pFile[0] = fopen(strCmpFileName[0].c_str(), "wb");
	pFile[1] = fopen(strCmpFileName[1].c_str(), "wb");

	if (pFile[0] == NULL || pFile[1] == NULL)
	{
		fclose(pFile[0]);
		fclose(pFile[1]);
		return 0;
	}

	NBT_Helper::Print(cpdInput[0], NBT_Print{ pFile[0] });
	NBT_Helper::Print(cpdInput[1], NBT_Print{ pFile[1] });

	fclose(pFile[0]);
	fclose(pFile[1]);

	printf("Cmp file gen!\n");
	return 0;
}


