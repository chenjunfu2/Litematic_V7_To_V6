#include "LitematicConversion.hpp"
#include "util/CodeTimer.hpp"

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <format>

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
		CodeTimer::Sleep(std::chrono::milliseconds(10));
		--u32TryCount;
	}

	//次数到上限直接返回空
	return std::string{};
}

struct MyCompoundSort
{
	static inline uint8_t u8Enabled;

	static void Reset()
	{
		u8Enabled = 2;
	}

	/// @brief 对给定的 Compound 对象进行排序，返回指向其元素的迭代器向量。
	/// @param cpdSort 需要排序的 Compound 对象。
	/// @return `std::vector<NBT_Type::Compound::Const_Iterator>`，其中迭代器按排序顺序排列。
	std::vector<NBT_Type::Compound::Const_Iterator> operator()(const NBT_Type::Compound &cpdSort)
	{
		if (u8Enabled == 0 || u8Enabled-- > 1)
		{
			return cpdSort.KeySortIt<>();
		}

		//第二层使用自定义排序
		std::vector<NBT_Type::Compound::Const_Iterator> vSortCompound{};
		vSortCompound.reserve(cpdSort.Size());
		for (auto it = cpdSort.begin(), end = cpdSort.end(); it != end; ++it)
		{
			vSortCompound.push_back(it);
		}

		std::sort(vSortCompound.begin(), vSortCompound.end(),
			[](const auto &l, const auto &r) -> bool
			{
				static std::unordered_map<NBT_Type::String, uint64_t> mapPriority =
				{
					{MU8STR("MinecraftDataVersion"),	0},
					{MU8STR("Version"),					1},
					{MU8STR("SubVersion"),				2},
					{MU8STR("Metadata"),				3},
					{MU8STR("Regions"),					4},
				};

				auto itL = mapPriority.find(l->first);
				auto itR = mapPriority.find(r->first);

				uint64_t u64LPriority = itL == mapPriority.end() ? (uint64_t)-1 : itL->second;
				uint64_t u64RPriority = itR == mapPriority.end() ? (uint64_t)-1 : itR->second;

				if (u64LPriority != u64RPriority)//都没找到才不成立
				{
					return u64LPriority < u64RPriority;
				}
				else
				{
					return l->first < r->first;
				}
			}
		);

		return vSortCompound;
	}
};

bool ConvertLitematicFile_V7_To_V6(const std::string &sV7FilePath)
{
	NBT_Type::Compound cpdV7Input{};
	NBT_Type::Compound cpdV6Output{};

	//从sV7FilePath读取到cpdV7Input
	{
		std::vector<uint8_t> vFileV7Stream{};
		if (!NBT_IO::ReadFile(sV7FilePath, vFileV7Stream))
		{
			printf("Unable to read stream from file!\n");
			return false;
		}

		//如果解压失败那么可能原先文件未压缩
		std::vector<uint8_t> vDataV7Stream{};
		if (!NBT_IO::DecompressDataNoThrow(vDataV7Stream, vFileV7Stream))
		{
			printf("Data may not be compressed, attempt to parse directly.\n");
			vDataV7Stream = std::move(vFileV7Stream);//尝试以未压缩流处理，而不是失败
		}

		if (!NBT_Reader::ReadNBT(vDataV7Stream, 0, cpdV7Input))
		{
			printf("Unable to parse data from stream!\n");
			return false;
		}
	}

	//从cpdV7Input转换到cpdV6Output
	std::string strErrMsg;
	if (!ConvertLitematicData_V7_To_V6(cpdV7Input, cpdV6Output, strErrMsg))
	{
		printf("Unable to convert v7_data to v6_data: [%s]\n", strErrMsg.c_str());
		return false;
	}

	//写出cpdV6Output到文件sV6FilePath
	{
		std::vector<uint8_t> vDataV6Stream{};
		MyCompoundSort::Reset();
		if (!NBT_Writer::WriteNBT<MyCompoundSort>(vDataV6Stream, 0, cpdV6Output))
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

		//压缩数据
		std::vector<uint8_t> vFileV6Stream{};
		if (!NBT_IO::CompressDataNoThrow(vFileV6Stream, vDataV6Stream))
		{
			printf("Unable to compress data stream!\n");
			return false;
		}

		//写入数据
		if (!NBT_IO::WriteFile(sV6FilePath, vFileV6Stream))
		{
			printf("Unable to write stream into file!\n");
			return false;
		}
	}

	printf("Convert Success!\n");
	return true;
}


