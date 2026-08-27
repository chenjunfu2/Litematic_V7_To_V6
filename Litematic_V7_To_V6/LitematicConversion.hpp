#pragma once

#include "RegionConversion.hpp"

#include <string>

#define V6_MINECRAFT_DATA_VERSION_END 3700//1.20.4->3700 检测值，至少大于此版本识别为v7
#define V6_MINECRAFT_DATA_VERSION_SAFE 3463//1.20->3463 写一个安全的值，以确保不要导致无法触发投影映射
#define NUM_TO_STR(x) #x
#define V6_LITEMATIC_VERSION 6
#define V6_LITEMATIC_SUBVERSION 1

bool ConvertLitematicData_V7_To_V6(NBT_Type::Compound &cpdV7Input, NBT_Type::Compound &cpdV6Output, std::string &strErrorMessage)
{
	//尝试获取唯一根部
	if (cpdV7Input.Size() != 1)
	{
		strErrorMessage = cpdV7Input.Empty() ?
			"Root node is missing! (expect exactly one)" :
			"Root node is ambiguous! (expect exactly one)";
		return false;
	}
	
	//必须要是Compound
	auto *pRoot = cpdV7Input.begin()->second.GetIfCompound();
	const auto &strRootName = cpdV7Input.begin()->first;
	if (pRoot == NULL)
	{
		strErrorMessage = "Root node is not a Compound type! (expected Compound)";
		return false;
	}

	//获取根部，并插入根部，最后获取根部引用
	auto &cpdV7DataRoot = *pRoot;
	auto &cpdV6DataRoot = cpdV6Output.PutCompound(strRootName, {}).first->second.GetCompound();//拷贝原先的V7根部名称

	//先处理版本信息
	auto *pMinecraftDataVersion = cpdV7DataRoot.HasInt(MU8STR("MinecraftDataVersion"));
	//auto *pVersion = cpdV7DataRoot.HasInt(MU8STR("Version"));
	//auto *pSubVersion = cpdV7DataRoot.HasInt(MU8STR("SubVersion"));

	//版本验证
	if (pMinecraftDataVersion == NULL || *pMinecraftDataVersion <= V6_MINECRAFT_DATA_VERSION_END)// || (pVersion == NULL || *pVersion <= V6_LITEMATIC_VERSION)//投影版本检测去除，仅关注MC版本
	{
		strErrorMessage = "MinecraftDataVersion Error! (must be > " NUM_TO_STR(V6_MINECRAFT_DATA_VERSION_END) ")";
		return false;
	}

	//基础数据
	auto *pMetadata = cpdV7DataRoot.HasCompound(MU8STR("Metadata"));
	if (pMetadata == NULL)
	{
		strErrorMessage = "Metadata not found!";
		return false;
	}

	//直接转移所有权，消除拷贝
	cpdV6DataRoot.PutCompound(MU8STR("Metadata"), std::move(*pMetadata));

	//设置基础版本信息
	cpdV6DataRoot.PutInt(MU8STR("MinecraftDataVersion"), V6_MINECRAFT_DATA_VERSION_SAFE);
	cpdV6DataRoot.PutInt(MU8STR("Version"), V6_LITEMATIC_VERSION);
	cpdV6DataRoot.PutInt(MU8STR("SubVersion"), V6_LITEMATIC_SUBVERSION);

	//获取
	auto *pRegions = cpdV7DataRoot.HasCompound(MU8STR("Regions"));
	if (pRegions == NULL)
	{
		strErrorMessage = "Regions not found!";
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
			strErrorMessage = "ProcessRegion fail!";
			return false;
		}
	}

	return true;
}
