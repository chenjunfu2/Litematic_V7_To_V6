#pragma once

#include <nbt_cpp/NBT_All.hpp>

#include <unordered_map>

//为了消拷贝，如果未进行映射，那么不会赋值，并返回false，否则赋值返回true
bool EntityIdMap(const NBT_Type::String &strEntityId, NBT_Type::String &strMappedEntityId, const NBT_Type::Int iV7McDataVersion)
{
	std::unordered_map<NBT_Type::String, NBT_Type::String> mapEntityId =
	{
		
	};

	auto itFind = mapEntityId.find(strEntityId);
	if (itFind == mapEntityId.end())
	{
		return false;
	}

	strMappedEntityId = itFind->second;
	return true;
}
