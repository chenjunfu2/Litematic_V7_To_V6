#pragma once

#include <nbt_cpp/NBT_All.hpp>

#include <unordered_map>

//为了消拷贝，如果未进行映射，那么不会赋值，并返回false，否则赋值返回true
bool BlockNameMap(const NBT_Type::String &strBlockName, NBT_Type::String& strMappedBlockName, const NBT_Type::Int iV7McDataVersion)
{
	std::unordered_map<NBT_Type::String, NBT_Type::String> mapBlockName =
	{
		{MU8STR("minecraft:short_grass"), MU8STR("minecraft:grass")},
	};

	auto itFind = mapBlockName.find(strBlockName);
	if (itFind == mapBlockName.end())
	{
		return false;
	}

	strMappedBlockName = itFind->second;
	return true;
}
