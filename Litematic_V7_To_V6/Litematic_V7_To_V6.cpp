#include "Process_Litematic_Data.hpp"

#include "util/CodeTimer.hpp"

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <thread>
#include <unordered_map>
#include <functional>

#define V6_MINECRAFT_DATA_VERSION 3700
#define V6_LITEMATIC_VERSION 6
#define V6_LITEMATIC_SUBVERSION 1

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


bool ProcessEntity(NBT_Type::Compound &cpdV7EntityData, NBT_Type::Compound &cpdV6EntityData)
{

}

bool ProcessTileEntity(NBT_Type::Compound &cpdV7TileEntityData, NBT_Type::Compound &cpdV6TileEntityData)
{
	FixTileEntityId(cpdV7TileEntityData);

	struct ValType
	{
	public:
		using TagProcessFunc_T = std::function<void(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)>;

	public:
		NBT_Type::String strNewKey;
		TagProcessFunc_T funcTagProcess;
		std::function<void(TagProcessFunc_T &funcTagProcess, NBT_Type::String &strNewKey, NBT_Type::Compound &cpdV6TileEntityData, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal)> funcProcess;

	public:
		void operator()(NBT_Type::Compound &cpdV6TileEntityData, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal)
		{
			return funcProcess(funcTagProcess, strNewKey, cpdV6TileEntityData, strV7TagKey, nodeV7TagVal);
		}
	};

	//通用处理
	auto funcDefaultProcess =
	[](ValType::TagProcessFunc_T &funcTagProcess, NBT_Type::String &strNewKey, NBT_Type::Compound &cpdV6TileEntityData, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal) -> void
	{
		NBT_Node nodeV6TagVal;
		funcTagProcess(nodeV7TagVal, nodeV6TagVal);
		cpdV6TileEntityData.Put(strNewKey, std::move(nodeV6TagVal));
	};

	//特殊处理
	auto funcJukeboxProcess =
	[](ValType::TagProcessFunc_T &funcTagProcess, NBT_Type::String &strNewKey, NBT_Type::Compound &cpdV6TileEntityData, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal) -> void
	{
		cpdV6TileEntityData.PutLong(MU8STR("RecordStartTick"), 0);
		cpdV6TileEntityData.PutLong(MU8STR("TickCount"), nodeV7TagVal.IsLong() ? nodeV7TagVal.GetLong() : 0);
		cpdV6TileEntityData.PutByte(MU8STR("IsPlaying"), 0);
	};

	std::unordered_map<NBT_Type::String, ValType> mapProccess =
	{
		{ MU8STR("Items"),						{ MU8STR("Items"),		ProcessItems,		funcDefaultProcess } },
		{ MU8STR("patterns"),					{ MU8STR("Patterns"),	ProcessPatterns,	funcDefaultProcess } },
		{ MU8STR("profile"),					{ MU8STR("SkullOwner"),	ProcessSkullProfile,funcDefaultProcess } },
		{ MU8STR("flower_pos"),					{ MU8STR("FlowerPos"),	ProcessFlowerPos,	funcDefaultProcess } },
		{ MU8STR("bees"),						{ MU8STR("Bees"),		ProcessBees,		funcDefaultProcess } },
		{ MU8STR("item"),						{ MU8STR("item"),		ProcessSingleItem,	funcDefaultProcess } },

		{ MU8STR("ticks_since_song_started"),	{ {},					{},					funcJukeboxProcess } }, //音符盒特殊处理

		{ MU8STR("RecordItem"),					{MU8STR("RecordItem"),	ProcessRecordItem,	funcDefaultProcess } },
		{ MU8STR("Book"),						{MU8STR("Book"),		ProcessBook,		funcDefaultProcess } },
		{ MU8STR("CustomName"),					{MU8STR("CustomName"),	ProcessCustomName,	funcDefaultProcess } },
		{ MU8STR("custom_name"),				{MU8STR("CustomName"),	ProcessCustomName,	funcDefaultProcess } },
	};

	for (auto &[itV7TagKey, itV7TagVal] : cpdV7TileEntityData)
	{
		//查找是否有匹配的处理过程
		auto itFind = mapProccess.find(itV7TagKey);
		if (itFind == mapProccess.end())
		{
			//不匹配直接移动处理
			cpdV6TileEntityData.Put(itV7TagKey, std::move(itV7TagVal));
			continue;
		}

		//进行处理
		ValType &vtProcess = itFind->second;
		vtProcess(cpdV6TileEntityData, itV7TagKey, itV7TagVal);
	}

	return true;
}

//V7到V6仅转换Entity与TileEntity，其余不变
bool ProcessRegion(NBT_Type::Compound &cpdV7RegionData, NBT_Type::Compound &cpdV6RegionData)
{
	//先转移不变数据，然后处理实体与方块实体

	//简易处理函数
	auto TransferDirectOptionalField = [&](const NBT_Type::String &strKey, NBT_TAG tag) -> bool
	{
		auto *pField = cpdV7RegionData.Has(strKey);
		if (pField != NULL &&
			(tag == NBT_TAG::ENUM_END || pField->GetTag() == tag))//ENUM_END表示接受任意类型，否则强匹配指定类型
		{
			cpdV6RegionData.Put(strKey, std::move(*pField));
			return true;
		}

		return false;
	};

	//这两个必须有，否则失败
	if (!TransferDirectOptionalField(MU8STR("Position"), NBT_TAG::Compound))
	{
		return false;
	}

	if (!TransferDirectOptionalField(MU8STR("Size"), NBT_TAG::Compound))
	{
		return false;
	}

	//下面的可能没有，没有则跳过插入
	(void)TransferDirectOptionalField(MU8STR("PendingBlockTicks"), NBT_TAG::List);
	(void)TransferDirectOptionalField(MU8STR("PendingFluidTicks"), NBT_TAG::List);
	(void)TransferDirectOptionalField(MU8STR("BlockStatePalette"), NBT_TAG::List);
	(void)TransferDirectOptionalField(MU8STR("BlockStates"), NBT_TAG::LongArray);

	//如果没有则跳过转换处理

	//实体处理
	do
	{
		auto *pEntities = cpdV7RegionData.HasList(MU8STR("Entities"));
		if (pEntities == NULL)
		{
			break;
		}

		auto it = cpdV6RegionData.PutList(MU8STR("Entities"), {}).first;
		auto &listV6EntityList = GetList(it->second);

		for (auto &nodeEntity : *pEntities)
		{
			auto itNode = listV6EntityList.AddBackCompound({}).first;

			if (!ProcessEntity(GetCompound(nodeEntity), GetCompound(*itNode)));
			{
				return false;
			}
		}
	} while (false);
	
	//方块实体处理
	do
	{
		auto *pTileEntities = cpdV7RegionData.HasList(MU8STR("TileEntities"));
		if (pTileEntities == NULL)
		{
			break;
		}

		auto it = cpdV6RegionData.PutList(MU8STR("TileEntities"), {}).first;
		auto &listV6EntityList = GetList(it->second);

		for (auto &nodeTileEntity : *pTileEntities)
		{
			auto itNode = listV6EntityList.AddBackCompound({}).first;

			if (!ProcessTileEntity(GetCompound(nodeTileEntity), GetCompound(*itNode)));
			{
				return false;
			}
		}
	} while (false);


	return true;
}


bool ConvertLitematicData_V7_To_V6(NBT_Type::Compound &cpdV7Input, NBT_Type::Compound &cpdV6Output)
{
	auto *pRoot = cpdV7Input.HasCompound(MU8STR(""));
	if (pRoot == NULL)
	{
		return false;
	}

	//获取根部，并插入根部，最后获取根部引用
	auto &cpdV7DataRoot = *pRoot;
	auto it = cpdV6Output.PutCompound(MU8STR(""), {}).first;
	auto &cpdV6DataRoot = GetCompound(it->second);

	//先处理版本信息
	auto *pMinecraftDataVersion = cpdV7DataRoot.HasInt(MU8STR("MinecraftDataVersion"));
	auto *pVersion = cpdV7DataRoot.HasInt(MU8STR("Version"));
	//auto *pSubVersion = cpdV7DataRoot.HasInt(MU8STR("SubVersion"));

	//版本验证
	if ((pMinecraftDataVersion != NULL && *pMinecraftDataVersion <= V6_MINECRAFT_DATA_VERSION) ||
		(pVersion != NULL && *pVersion <= V6_MINECRAFT_DATA_VERSION))
	{
		return false;
	}

	//基础数据
	auto *pMetadata = cpdV7DataRoot.HasCompound(MU8STR("Metadata"));
	if (pMetadata == NULL)
	{
		return false;
	}

	//直接转移所有权，消除拷贝
	cpdV6DataRoot.PutCompound(MU8STR("Metadata"), std::move(*pMetadata));

	//设置基础版本信息
	cpdV6DataRoot.PutInt(MU8STR("MinecraftDataVersion"), V6_MINECRAFT_DATA_VERSION);
	cpdV6DataRoot.PutInt(MU8STR("Version"), V6_LITEMATIC_VERSION);
	cpdV6DataRoot.PutInt(MU8STR("SubVersion"), V6_LITEMATIC_SUBVERSION);

	//获取
	auto *pRegions = cpdV7DataRoot.HasCompound(MU8STR("Regions"));
	if (pRegions == NULL)
	{
		return false;
	}

	//插入选区根
	auto it2 = cpdV6DataRoot.PutCompound(MU8STR("Regions"), {}).first;
	auto &cpdV6Regions = GetCompound(it2->second);

	//遍历选区
	for (auto &[sV7RegionName, nodeV7RegionData] : *pRegions)
	{
		auto itNew = cpdV6Regions.PutCompound(sV7RegionName, {}).first;
		auto &cpdNewV6RegionData = GetCompound(itNew->second);

		if (!ProcessRegion(GetCompound(nodeV7RegionData), cpdNewV6RegionData))
		{
			return false;
		}
	}

	return true;
}


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

		if (!NBT_Reader::ReadNBT(vDataV7Stream, cpdV7Input))
		{
			printf("Unable to parse data from stream!\n");
			return false;
		}
	}

	//从cpdV7Input转换到cpdV6Output
	if (!ConvertLitematicData_V7_To_V6(cpdV7Input, cpdV6Output))
	{
		printf("Unable to convert v7_data to v6_data!\n");
		return false;
	}

	//写出cpdV6Output到文件sV6FilePath
	{
		std::vector<uint8_t> vDataV6Stream{};
		if (!NBT_Writer::WriteNBT(vDataV6Stream, cpdV6Output))
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

	return true;
}


