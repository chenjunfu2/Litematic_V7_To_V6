#pragma once

#include "BaseConversion.hpp"

void ProcessBlockPosExternal(const NBT_Type::String &strPosPerfix, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
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

	cpdV6TagData.PutInt(strPosPerfix + MU8STR("X"), iarrBlockPos[0]);
	cpdV6TagData.PutInt(strPosPerfix + MU8STR("Y"), iarrBlockPos[1]);
	cpdV6TagData.PutInt(strPosPerfix + MU8STR("Z"), iarrBlockPos[2]);

	return;
}

void ProcessEntityItems(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion, size_t szSlotSize)
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

void ProcessEntity(NBT_Type::Compound &cpdV7EntityData, NBT_Type::Compound &cpdV6EntityData, const NBT_Type::Int iV7McDataVersion)
{
	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;
	using std::placeholders::_4;

	const static std::unordered_map<NBT_Type::String, MapValFunc_T> mapProccess =
	{
		{ MU8STR("has_egg"),			std::bind(RenameProcess,			MU8STR("HasEgg"),			_1, _2, _3, _4) },
		{ MU8STR("life_ticks"),			std::bind(RenameProcess,			MU8STR("LifeTicks"),		_1, _2, _3, _4) },
		{ MU8STR("size"),				std::bind(RenameProcess,			MU8STR("Size"),				_1, _2, _3, _4) },
		{ MU8STR("fall_distance"),		std::bind(RenameProcess,			MU8STR("FallDistance"),		_1, _2, _3, _4) },

		{ MU8STR("anchor_pos"),			std::bind(ProcessBlockPosExternal,	MU8STR("A"),				_1, _2, _3, _4) },
		{ MU8STR("block_pos"),			std::bind(ProcessBlockPosExternal,	MU8STR("Tile"),				_1, _2, _3, _4) },
		{ MU8STR("bound_pos"),			std::bind(ProcessBlockPosExternal,	MU8STR("Bound"),			_1, _2, _3, _4) },
		{ MU8STR("home_pos"),			std::bind(ProcessBlockPosExternal,	MU8STR("HomePos"),			_1, _2, _3, _4) },
		{ MU8STR("sleeping_pos"),		std::bind(ProcessBlockPosExternal,	MU8STR("Sleeping"),			_1, _2, _3, _4) },

		{ MU8STR("attributes"),			std::bind(DefaultProcess,			MU8STR("Attributes"),	ProcessAttributes,	_1, _2, _3, _4) },
		{ MU8STR("flower_pos"),			std::bind(DefaultProcess,			MU8STR("FlowerPos"),	ProcessBlockPos,	_1, _2, _3, _4) },
		{ MU8STR("hive_pos"),			std::bind(DefaultProcess,			MU8STR("HivePos"),		ProcessBlockPos,	_1, _2, _3, _4) },
		{ MU8STR("Item"),				std::bind(DefaultProcess,			MU8STR("Item"),			ProcessSingleItem,	_1, _2, _3, _4) },
		{ MU8STR("Items"),				std::bind(DefaultProcess,			MU8STR("Items"),		ProcessItems,		_1, _2, _3, _4) },

		{ MU8STR("ArmorItems"),			std::bind(DefaultProcess,			MU8STR("ArmorItems"),	(TagProcessFunc_T)std::bind(ProcessEntityItems, _1, _2, _3, 4),		_1, _2, _3, _4) },
		{ MU8STR("HandItems"),			std::bind(DefaultProcess,			MU8STR("HandItems"),	(TagProcessFunc_T)std::bind(ProcessEntityItems, _1, _2, _3, 2),		_1, _2, _3, _4) },
		{ MU8STR("Inventory"),			std::bind(DefaultProcess,			MU8STR("Inventory"),	(TagProcessFunc_T)std::bind(ProcessEntityItems, _1, _2, _3, 1),		_1, _2, _3, _4) },

		{ MU8STR("equipment"),			ProcessEntityEquipment },
		{ MU8STR("drop_chances"),		ProcessEntityDropChances },
	};


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

	return;
}
