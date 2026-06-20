#pragma once

#include "RegionConversion.hpp"

#include <string>

#define V6_MINECRAFT_DATA_VERSION 3700
#define V6_LITEMATIC_VERSION 6
#define V6_LITEMATIC_SUBVERSION 1

bool ConvertLitematicData_V7_To_V6(NBT_Type::Compound &cpdV7Input, NBT_Type::Compound &cpdV6Output, std::string &strErrorMessage)
{
	auto *pRoot = cpdV7Input.HasCompound(MU8STR(""));
	if (pRoot == NULL)
	{
		strErrorMessage = "Root Compound not found!\n";
		return false;
	}

	//获取根部，并插入根部，最后获取根部引用
	auto &cpdV7DataRoot = *pRoot;
	auto &cpdV6DataRoot = cpdV6Output.PutCompound(MU8STR(""), {}).first->second.GetCompound();

	//先处理版本信息
	auto *pMinecraftDataVersion = cpdV7DataRoot.HasInt(MU8STR("MinecraftDataVersion"));
	//auto *pVersion = cpdV7DataRoot.HasInt(MU8STR("Version"));
	//auto *pSubVersion = cpdV7DataRoot.HasInt(MU8STR("SubVersion"));

	//版本验证
	if (pMinecraftDataVersion == NULL || *pMinecraftDataVersion <= V6_MINECRAFT_DATA_VERSION)// || (pVersion == NULL || *pVersion <= V6_LITEMATIC_VERSION)//投影版本检测去除，仅关注MC版本
	{
		strErrorMessage = "MinecraftDataVersion Error!\n";
		return false;
	}

	//基础数据
	auto *pMetadata = cpdV7DataRoot.HasCompound(MU8STR("Metadata"));
	if (pMetadata == NULL)
	{
		strErrorMessage = "Metadata not found!\n";
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
		strErrorMessage = "Regions not found!\n";
		return false;
	}

	//插入选区根
	auto &cpdV6Regions = cpdV6DataRoot.PutCompound(MU8STR("Regions"), {}).first->second.GetCompound();

	//遍历选区
	for (auto &[sV7RegionName, nodeV7RegionData] : *pRegions)
	{
		auto &cpdNewV6RegionData = cpdV6Regions.PutCompound(sV7RegionName, {}).first->second.GetCompound();
		if (!ProcessRegion(GetCompound(nodeV7RegionData), cpdNewV6RegionData, *pMinecraftDataVersion))
		{
			strErrorMessage = "ProcessRegion fail!\n";
			return false;
		}
	}

	return true;
}
