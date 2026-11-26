#pragma once

#include "BaseConversion.hpp"
#include "EntityMappings.hpp"

#include <algorithm>

template<auto PosPerfix, uint8_t u8PosTypeIndex>
requires(IsMU8StringArray<decltype(PosPerfix)>)
consteval NBT_Type::String::View MakePosStr(void)
{
	if (u8PosTypeIndex >= 3)
	{
		throw std::out_of_range("u8PosTypeIndex out of range");
	}

	return MUTF8_Tool_Internal::ToStringView
	<
		[]() consteval -> auto
		{
			constexpr size_t szNewSize = PosPerfix.size() + 1;//PosPerfix 无0结尾字符串，加1预留
			std::array<MUTF8_Char_Type, szNewSize> strRet{};//0初始化
			std::ranges::copy(PosPerfix, &strRet[0]);//拷贝前面的部分
			strRet[szNewSize - 1] = MU8RAW("XYZ")[u8PosTypeIndex];//添加末尾值
			return strRet;
		}(),
		NBT_Type::String::View
	>();
}


template<auto PosPerfix>
requires(IsMU8StringArray<decltype(PosPerfix)>)
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

void ProcessEntityModifiers(NBT_Node &V7Tag, NBT_Node &V6Tag, const NBT_Type::String &strEntityId, const NBT_Type::Int iV7McDataVersion)
{
	if (!V7Tag.IsCompound())
	{
		V6Tag = std::move(V7Tag);
		return;
	}

	auto &cpdV7 = V7Tag.GetCompound();
	auto &cpdV6 = V6Tag.SetCompound();

	if (auto *pId = cpdV7.HasString(MU8STRV("id")); pId != NULL)
	{
		AttributeInfo info = AttributeInfoMap(*pId, strEntityId);//顺手的事

		if (info.bIsRandomUuid)
		{
			cpdV6.PutString(MU8STRV("Name"), std::move(info.name));//name被映射，uuid随机生成
			UUID uuid = NameUuidFromBytes(pId->data(), pId->size());//从原始id生成uuid，确定值
			cpdV6.PutIntArray(MU8STRV("UUID"), UUID2IntArray(uuid));//放入uuid
		}
		else
		{
			cpdV6.PutString(MU8STRV("Name"), std::move(*pId));//uuid被映射，name直接消费
			cpdV6.PutIntArray(MU8STRV("UUID"), UUID2IntArray(info.uuid));//放入映射uuid
		}
	}

	TryRenameMove<NBT_TAG::Double>(cpdV7, MU8STRV("amount"), cpdV6, MU8STRV("Amount"));

	TryRenameTransfer<NBT_TAG::String>(cpdV7, MU8STRV("operation"), cpdV6, MU8STRV("Operation"),
		[](NBT_Type::String &v7Val, NBT_Node &v6Val) -> bool
		{
			v6Val.SetInt(AttributeOperationMap(v7Val));
			return true;
		}
	);

	return;
}

void ProcessAttributesImpl(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::String &strEntityId, const NBT_Type::Int iV7McDataVersion)
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

		auto &cpdV7Entry = itV7Entry.GetCompound();
		NBT_Type::Compound cpdV6Entry;

		TryRenameTransfer<NBT_TAG::String>(cpdV7Entry, MU8STRV("id"), cpdV6Entry, MU8STRV("Name"),
			[](NBT_Type::String &v7Val, NBT_Node &v6Val) -> bool
			{
				v6Val.SetString(AttributeNameMap(v7Val));
				return true;
			}
		);

		TryRenameMove<NBT_TAG::Double>(cpdV7Entry, MU8STR("base"), cpdV6Entry, MU8STR("Base"), 0.0);

		TryRenameTransfer<NBT_TAG::List>(cpdV7Entry, MU8STRV("modifiers"), cpdV6Entry, MU8STR("Modifiers"),
			[&strEntityId, iV7McDataVersion](NBT_Type::List &v7Val, NBT_Node &v6Val) -> bool
			{
				NBT_Type::List &listV6Modifiers = v6Val.SetList();
				for (auto &v7It : v7Val)
				{
					NBT_Node v6Tmp{};
					ProcessEntityModifiers(v7It, v6Tmp, strEntityId, iV7McDataVersion);
					listV6Modifiers.AddBack(std::move(v6Tmp));
				}

				return !listV6Modifiers.Empty();//非空返回true被插入，否则丢弃
			}
		);

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
}

void ProcessAttributes(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	//attributes -> Attributes
	NBT_Type::String strEmpty{};
	auto *pEntityId = cpdV6TagData.HasString(MU8STRV("id"));
	if (pEntityId == NULL)
	{
		pEntityId = &strEmpty;
	}

	NBT_Node nodeV6TagVal;
	ProcessAttributesImpl(nodeV7TagVal, nodeV6TagVal, *pEntityId, iV7McDataVersion);
	cpdV6TagData.Put(MU8STR("Attributes"), std::move(nodeV6TagVal));
};


void ProcessEntity(NBT_Type::Compound &cpdV7EntityData, NBT_Type::Compound &cpdV6EntityData, const NBT_Type::Int iV7McDataVersion)
{
	const static std::unordered_map<NBT_Type::String, MapProcessFunc_P> mapProccess =
	{
		{ MU8STR("custom_name"),		RenameAndProcess<MU8RAW("CustomName"),	ProcessTextComponentTag> },
		{ MU8STR("CustomName"),			RenameAndProcess<MU8RAW("CustomName"),	ProcessTextComponentTag> },

		{ MU8STR("has_egg"),			RenameOnly<MU8RAW("HasEgg")> },
		{ MU8STR("life_ticks"),			RenameOnly<MU8RAW("LifeTicks")> },
		{ MU8STR("size"),				RenameOnly<MU8RAW("Size")> },
		{ MU8STR("fall_distance"),		RenameOnly<MU8RAW("FallDistance")> },

		{ MU8STR("anchor_pos"),			ProcessBlockPosExternal<MU8RAW("A")> },
		{ MU8STR("block_pos"),			ProcessBlockPosExternal<MU8RAW("Tile")> },
		{ MU8STR("bound_pos"),			ProcessBlockPosExternal<MU8RAW("Bound")> },
		{ MU8STR("home_pos"),			ProcessBlockPosExternal<MU8RAW("HomePos")> },
		{ MU8STR("sleeping_pos"),		ProcessBlockPosExternal<MU8RAW("Sleeping")> },

		{ MU8STR("flower_pos"),			RenameAndProcess<MU8RAW("FlowerPos"),	ProcessBlockPos> },
		{ MU8STR("hive_pos"),			RenameAndProcess<MU8RAW("HivePos"),		ProcessBlockPos> },
		{ MU8STR("Item"),				RenameAndProcess<MU8RAW("Item"),			ProcessSingleItem> },
		{ MU8STR("Items"),				RenameAndProcess<MU8RAW("Items"),		ProcessItems> },

		{ MU8STR("ArmorItems"),			RenameAndProcess<MU8RAW("ArmorItems"),	ProcessEntityItems<4>> },
		{ MU8STR("HandItems"),			RenameAndProcess<MU8RAW("HandItems"),	ProcessEntityItems<2>> },
		{ MU8STR("Inventory"),			RenameAndProcess<MU8RAW("Inventory"),	ProcessEntityItems<1>> },

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
