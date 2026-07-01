#pragma once

#include "BaseConversion.hpp"
#include "BlockConversion.hpp"
#include "ItemConversion.hpp"
#include "TileEntityConversion.hpp"
#include "EntityConversion.hpp"

bool ProcessRegion(NBT_Type::Compound &cpdV7RegionData, NBT_Type::Compound &cpdV6RegionData, const NBT_Type::Int iV7McDataVersion)
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
		printf("Position not found!\n");
		return false;
	}

	if (!TransferDirectOptionalField(MU8STR("Size"), NBT_TAG::Compound))
	{
		printf("Size not found!\n");
		return false;
	}

	//下面的可能没有，没有则跳过插入
	(void)TransferDirectOptionalField(MU8STR("PendingBlockTicks"), NBT_TAG::List);
	(void)TransferDirectOptionalField(MU8STR("PendingFluidTicks"), NBT_TAG::List);
	(void)TransferDirectOptionalField(MU8STR("BlockStates"), NBT_TAG::LongArray);

	//如果没有则跳过转换处理

	//方块调色板处理（方块可能有同名注册实体，根据转换映射注册表决定）
	do
	{
		auto *pBlockStatePalette = cpdV7RegionData.HasList(MU8STR("BlockStatePalette"));
		if (pBlockStatePalette == NULL)
		{
			break;
		}

		auto &listV6BlockStatePalette = cpdV6RegionData.PutList(MU8STR("BlockStatePalette"), {}).first->second.GetList();
		for (auto &nodeBlockState : *pBlockStatePalette)
		{
			auto *pCpdBlockState = nodeBlockState.GetIfCompound();
			if (pCpdBlockState == NULL)
			{
				listV6BlockStatePalette.AddBack(std::move(nodeBlockState));
				continue;
			}

			auto &cpdNode = listV6BlockStatePalette.AddBackCompound({}).GetCompound();
			ProcessBlock(*pCpdBlockState, cpdNode, iV7McDataVersion);
		}
	} while (false);

	//实体处理（包含物品处理，备注：实体内可能包含物品，物品实体本身就算一种内部包含物品的实体，根据NBT标签的形式来看可以这么分类）
	do
	{
		auto *pEntities = cpdV7RegionData.HasList(MU8STR("Entities"));
		if (pEntities == NULL)
		{
			break;
		}

		auto &listV6EntityList = cpdV6RegionData.PutList(MU8STR("Entities"), {}).first->second.GetList();
		for (auto &nodeEntity : *pEntities)
		{
			auto *pCpdEntity = nodeEntity.GetIfCompound();
			if (pCpdEntity == NULL)
			{
				listV6EntityList.AddBack(std::move(nodeEntity));
				continue;
			}

			auto &cpdNode = listV6EntityList.AddBackCompound({}).GetCompound();
			ProcessEntity(*pCpdEntity, cpdNode, iV7McDataVersion);
		}
	} while (false);

	//方块实体处理（包含物品处理，备注：方块实体内可能包含物品）
	do
	{
		auto *pTileEntities = cpdV7RegionData.HasList(MU8STR("TileEntities"));
		if (pTileEntities == NULL)
		{
			break;
		}

		auto &listV6TileEntityList = cpdV6RegionData.PutList(MU8STR("TileEntities"), {}).first->second.GetList();
		for (auto &nodeTileEntity : *pTileEntities)
		{
			auto *pCpdTileEntity = nodeTileEntity.GetIfCompound();
			if (pCpdTileEntity == NULL)
			{
				listV6TileEntityList.AddBack(std::move(nodeTileEntity));
				continue;
			}

			auto &cpdNode = listV6TileEntityList.AddBackCompound({}).GetCompound();
			ProcessTileEntity(*pCpdTileEntity, cpdNode, iV7McDataVersion);
		}
	} while (false);


	return true;
}
