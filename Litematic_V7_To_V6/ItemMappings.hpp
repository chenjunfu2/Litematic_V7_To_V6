#pragma once

#include <nbt_cpp/NBT_All.hpp>

#include <unordered_map>

//为了消拷贝，如果未进行映射，那么不会赋值，并返回false，否则赋值返回true
bool ItemIdMap(const NBT_Type::String &strItemId, NBT_Type::String &strMappedItemId, const NBT_Type::Int iV7McDataVersion)
{
	std::unordered_map<NBT_Type::String, NBT_Type::String> mapItemId =
	{
		{MU8STR("minecraft:short_grass"), MU8STR("minecraft:grass")},
		{MU8STR("minecraft:turtle_scute"), MU8STR("minecraft:scute")},
	};

	auto itFind = mapItemId.find(strItemId);
	if (itFind == mapItemId.end())
	{
		return false;
	}

	strMappedItemId = itFind->second;
	return true;
}

