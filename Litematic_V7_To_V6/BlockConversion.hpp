#pragma once

#include "BaseConversion.hpp"
#include "BlockMappings.hpp"

void ProcessBlock(NBT_Type::Compound &cpdV7BlockData, NBT_Type::Compound &cpdV6BlockData, const NBT_Type::Int iV7McDataVersion)
{
	//目前，仅有name-name映射，没有方块state的相关处理，暂时不使用复杂方案，仅对name进行映射，有需要后续再改
	cpdV6BlockData = std::move(cpdV7BlockData);
	auto *pBlockName = cpdV6BlockData.HasString(MU8STR("Name"));
	if (pBlockName != NULL)
	{
		(void)BlockNameMap(*pBlockName, *pBlockName, iV7McDataVersion);//直接映射并替换，如果失败不会进行替换
	}
}
