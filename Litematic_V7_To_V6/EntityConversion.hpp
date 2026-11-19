#pragma once

#include "BaseConversion.hpp"
#include "EntityMappings.hpp"

#include <algorithm>

template<MUTF8_Tool_Internal::StringLiteral PosPerfix, uint8_t u8PosTypeIndex>
requires(IsU8StringLiteral<decltype(PosPerfix)>)
consteval NBT_Type::String::View MakePosStr(void)
{
	if (u8PosTypeIndex >= 3)
	{
		throw std::out_of_range("u8PosTypeIndex out of range");
	}

	return MUTF8_Tool<>::U8ToMU8<
		[]() consteval -> auto
		{
			constexpr size_t szNewSize = PosPerfix.size();//PosPerfix size包含末尾0
			char8_t strRet[szNewSize + 1]{};//末尾加0
			std::ranges::copy(PosPerfix, &strRet[0]);
			strRet[szNewSize - 1] = U8STR("XYZ")[u8PosTypeIndex];//覆盖拷贝来的末尾0（有预留空间）
			return MUTF8_Tool_Internal::StringLiteral{strRet};
		}()
	>();
}


template<MUTF8_Tool_Internal::StringLiteral PosPerfix>
requires(IsU8StringLiteral<decltype(PosPerfix)>)
void ProcessBlockPosExternal(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsIntArray())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto &iarrBlockPos = nodeV7TagVal.GetIntArray();
	if (iarrBlockPos.size() != 3)
	{
		return;
	}

	cpdV6TagData.PutInt(MakePosStr<PosPerfix, 0>(), iarrBlockPos[0]);
	cpdV6TagData.PutInt(MakePosStr<PosPerfix, 1>(), iarrBlockPos[1]);
	cpdV6TagData.PutInt(MakePosStr<PosPerfix, 2>(), iarrBlockPos[2]);

	return;
}

template<size_t SlotSize>
void ProcessEntityItems(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsList())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &listV7 = nodeV7Tag.GetList();
	auto &listV6 = nodeV6Tag.SetList();

	for (auto &itV7Entry : listV7)
	{
		if (!itV7Entry.IsCompound())
		{
			continue;
		}

		NBT_Node nodeV6Entry;
		ProcessSingleItem(itV7Entry, nodeV6Entry, iV7McDataVersion);
		if (!nodeV6Entry.IsCompound())
		{
			continue;
		}

		listV6.AddBack(std::move(nodeV6Entry));
	}

	size_t szSlotSize = SlotSize;
	if (listV6.Size() < szSlotSize)
	{
		szSlotSize -= listV6.Size();
		while (szSlotSize-- > 0)
		{
			listV6.AddBackCompound({});
		}
	}

	return;
}

void ProcessEntityEquipment(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto &cpdV7Tag = nodeV7TagVal.GetCompound();

	NBT_Type::List listHandItems{ NBT_Type::Compound{}, NBT_Type::Compound{} };
	NBT_Type::List listArmorItems{ NBT_Type::Compound{}, NBT_Type::Compound{}, NBT_Type::Compound{}, NBT_Type::Compound{} };

	for (auto &[strV7Key, nodeV7Val] : cpdV7Tag)
	{
		switch (GetHandArmorSlot(strV7Key))
		{
		case HandArmorSlot::mainhand:
			ProcessSingleItem(nodeV7Val, listHandItems[0], iV7McDataVersion);
			break;
		case HandArmorSlot::offhand:
			ProcessSingleItem(nodeV7Val, listHandItems[1], iV7McDataVersion);
			break;
		case HandArmorSlot::feet:
			ProcessSingleItem(nodeV7Val, listArmorItems[0], iV7McDataVersion);
			break;
		case HandArmorSlot::legs:
			ProcessSingleItem(nodeV7Val, listArmorItems[1], iV7McDataVersion);
			break;
		case HandArmorSlot::chest:
			ProcessSingleItem(nodeV7Val, listArmorItems[2], iV7McDataVersion);
			break;
		case HandArmorSlot::head:
			ProcessSingleItem(nodeV7Val, listArmorItems[3], iV7McDataVersion);
			break;
		case HandArmorSlot::body:
			{
				//注释来自投影：Why is this duplicated in 1.20.4?  the world may never know...
				NBT_Node nodeV6Body;
				ProcessSingleItem(nodeV7Val, nodeV6Body, iV7McDataVersion);
				listArmorItems[2] = nodeV6Body;//拷贝一份
				cpdV6TagData.Put(MU8STR("ArmorItem"), std::move(nodeV6Body));
			}
			break;
		case HandArmorSlot::saddle:
			{
				NBT_Node nodeV6Saddle;
				ProcessSingleItem(nodeV7Val, nodeV6Saddle, iV7McDataVersion);
				cpdV6TagData.Put(MU8STR("SaddleItem"), std::move(nodeV6Saddle));
			}
			break;
		case HandArmorSlot::unknown:
			continue;
			break;
		default:
			continue;
			break;
		}
	}

	cpdV6TagData.PutList(MU8STR("HandItems"), std::move(listHandItems));
	cpdV6TagData.PutList(MU8STR("ArmorItems"), std::move(listArmorItems));

	return;
}

void ProcessEntityDropChances(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto &cpdV7Tag = nodeV7TagVal.GetCompound();

	NBT_Type::List listHandDrops{ NBT_Type::Compound{}, NBT_Type::Compound{} };
	NBT_Type::List listArmorDrops{ NBT_Type::Compound{}, NBT_Type::Compound{}, NBT_Type::Compound{}, NBT_Type::Compound{} };

	for (auto &[strV7Key, nodeV7Val] : cpdV7Tag)
	{
		switch (GetHandArmorSlot(strV7Key))
		{
		case HandArmorSlot::mainhand:
			listHandDrops[0] = std::move(nodeV7Val);
			break;
		case HandArmorSlot::offhand:
			listHandDrops[1] = std::move(nodeV7Val);
			break;
		case HandArmorSlot::feet:
			listArmorDrops[0] = std::move(nodeV7Val);
			break;
		case HandArmorSlot::legs:
			listArmorDrops[1] = std::move(nodeV7Val);
			break;
		case HandArmorSlot::chest:
			listArmorDrops[2] = std::move(nodeV7Val);
			break;
		case HandArmorSlot::head:
			listArmorDrops[3] = std::move(nodeV7Val);
			break;
			//case HandArmorSlot::body: //ignored
			//	break;
			//case HandArmorSlot::saddle: //ignored
			//	break;
		case HandArmorSlot::unknown:
			continue;
			break;
		default:
			continue;
			break;
		}
	}

	cpdV6TagData.PutList(MU8STR("HandDropChances"), std::move(listHandDrops));
	cpdV6TagData.PutList(MU8STR("ArmorDropChances"), std::move(listArmorDrops));

	return;
}

bool ProcessBoatMappings(
	const NBT_Type::String &strEntityId,
	NBT_Type::String &strMappedId,
	NBT_Type::String::View &strvBoatType,
	NBT_Type::String::View strvBoatSuffix,
	NBT_Type::String::View strvChestBoatSuffix)
{
	static constexpr auto strvMcPrefix = MU8STRV("minecraft:");
	static constexpr auto strvBoat = MU8STRV("minecraft:boat");
	static constexpr auto strvChestBoat = MU8STRV("minecraft:chest_boat");

	if (!strEntityId.ends_with(strvBoatSuffix) || !strEntityId.starts_with(strvMcPrefix))
	{
		return false;
	}

	strvBoatType = NBT_Type::String::View{ strEntityId };
	strvBoatType.remove_prefix(strvMcPrefix.size());

	if (strEntityId.ends_with(strvChestBoatSuffix))
	{
		strvBoatType.remove_suffix(strvChestBoatSuffix.size());
		strMappedId = strvChestBoat;
	}
	else
	{
		strvBoatType.remove_suffix(strvBoatSuffix.size());
		strMappedId = strvBoat;
	}

	return true;
}

//特殊，需要保住原始id不能被搞坏，并且在后面用到
void ProcessIdMappings(NBT_Type::String &strIdVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	//处理船、筏映射
	do
	{
		NBT_Type::String::View strvBoatType;
		if (!ProcessBoatMappings(strIdVal, strIdVal, strvBoatType, MU8STRV("_boat"), MU8STRV("_chest_boat")) &&
			!ProcessBoatMappings(strIdVal, strIdVal, strvBoatType, MU8STRV("_raft"), MU8STRV("_chest_raft")))
		{
			break;
		}

		//至少有一个映射成功
		//插入"Type":strvBoatType
		cpdV6TagData.PutString(MU8STR("Type"), strvBoatType);

		return;
	} while (false);

	//处理其它映射
	EntityIdMap(strIdVal, strIdVal, iV7McDataVersion);
	return;
}

NBT_Type::String MapEntityTypeIdForDisplay(NBT_Type::String &strId, const NBT_Type::Int iV7McDataVersion)
{
	do
	{
		static constexpr auto strvBoatSuffix = MU8STRV("_boat");
		static constexpr auto strvRaftSuffix = MU8STRV("_raft");

		static constexpr auto strvChestBoatSuffix = MU8STRV("_chest_boat");
		static constexpr auto strvChestRaftSuffix = MU8STRV("_chest_raft");

		NBT_Type::String::View strvBoatType;
		if (!ProcessBoatMappings(strId, strId, strvBoatType, strvBoatSuffix, strvChestBoatSuffix) &&
			!ProcessBoatMappings(strId, strId, strvBoatType, strvRaftSuffix, strvChestRaftSuffix))
		{
			break;
		}

		return std::move(strId);
	} while (false);

	(void)EntityIdMap(strId, strId, iV7McDataVersion);
	return std::move(strId);
}


void ProcessEntity(NBT_Type::Compound &cpdV7EntityData, NBT_Type::Compound &cpdV6EntityData, const NBT_Type::Int iV7McDataVersion)
{
	const static std::unordered_map<NBT_Type::String, MapProcessFunc_P> mapProccess =
	{
		{ MU8STR("custom_name"),		RenameAndProcess<U8STR("CustomName"),	ProcessTextComponentTag> },
		{ MU8STR("CustomName"),			RenameAndProcess<U8STR("CustomName"),	ProcessTextComponentTag> },

		{ MU8STR("has_egg"),			RenameOnly<U8STR("HasEgg")> },
		{ MU8STR("life_ticks"),			RenameOnly<U8STR("LifeTicks")> },
		{ MU8STR("size"),				RenameOnly<U8STR("Size")> },
		{ MU8STR("fall_distance"),		RenameOnly<U8STR("FallDistance")> },

		{ MU8STR("anchor_pos"),			ProcessBlockPosExternal<U8STR("A")> },
		{ MU8STR("block_pos"),			ProcessBlockPosExternal<U8STR("Tile")> },
		{ MU8STR("bound_pos"),			ProcessBlockPosExternal<U8STR("Bound")> },
		{ MU8STR("home_pos"),			ProcessBlockPosExternal<U8STR("HomePos")> },
		{ MU8STR("sleeping_pos"),		ProcessBlockPosExternal<U8STR("Sleeping")> },

		{ MU8STR("flower_pos"),			RenameAndProcess<U8STR("FlowerPos"),	ProcessBlockPos> },
		{ MU8STR("hive_pos"),			RenameAndProcess<U8STR("HivePos"),		ProcessBlockPos> },
		{ MU8STR("Item"),				RenameAndProcess<U8STR("Item"),			ProcessSingleItem> },
		{ MU8STR("Items"),				RenameAndProcess<U8STR("Items"),		ProcessItems> },

		{ MU8STR("ArmorItems"),			RenameAndProcess<U8STR("ArmorItems"),	ProcessEntityItems<4>> },
		{ MU8STR("HandItems"),			RenameAndProcess<U8STR("HandItems"),	ProcessEntityItems<2>> },
		{ MU8STR("Inventory"),			RenameAndProcess<U8STR("Inventory"),	ProcessEntityItems<1>> },

		{ MU8STR("attributes"),			ProcessAttributes },
		{ MU8STR("equipment"),			ProcessEntityEquipment },
		{ MU8STR("drop_chances"),		ProcessEntityDropChances },
	};


	//首先处理id
	if(auto *pEntityId = cpdV7EntityData.Has(MU8STR("id")); pEntityId != NULL)
	{
		cpdV6EntityData.Put(MU8STR("id"), std::move(*pEntityId));//先移入V6以便其它处理例程使用
		cpdV7EntityData.Remove(MU8STR("id"));//务必删除，防止下方重复使用
	}

	for (auto &[itV7TagKey, itV7TagVal] : cpdV7EntityData)
	{
		//查找是否有匹配的处理过程
		auto itFind = mapProccess.find(itV7TagKey);
		if (itFind == mapProccess.end())
		{
			//不匹配直接移动处理
			cpdV6EntityData.Put(itV7TagKey, std::move(itV7TagVal));
			continue;
		}

		//进行处理
		auto &funcProcess = itFind->second;
		funcProcess(itV7TagKey, itV7TagVal, cpdV6EntityData, iV7McDataVersion);
	}

	//最后处理id映射
	if (auto *pEntityId = cpdV6EntityData.HasString(MU8STR("id")); pEntityId != NULL)
	{
		ProcessIdMappings(*pEntityId, cpdV6EntityData, iV7McDataVersion);
	}

	return;
}
