#pragma once

#include "BaseConversion.hpp"

void FixTileEntityId(NBT_Type::Compound &cpdTileEntity, const NBT_Type::Int iV7McDataVersion)
{
	if (cpdTileEntity.ContainsString(MU8STR("id")))
	{
		return;
	}

	auto *pId = cpdTileEntity.HasString(MU8STR("Id"));
	if (pId != NULL)
	{
		cpdTileEntity.PutString(MU8STR("id"), std::move(*pId));//转化为小写id
		cpdTileEntity.Remove(MU8STR("Id"));//删除
		return;
	}

	//找不到Id或者Id类型不是字符串，通过其它项猜测类型
	struct GuessNode
	{
		NBT_Type::String strContains;//主条件
		std::vector<NBT_Type::String> listAdditional;//附加条件
		NBT_Type::String strIdGuess;
	};

	const static std::vector<GuessNode> listIdGuess =
	{
		{MU8STR("Bees"),					{},														MU8STR("minecraft:beehive")},
		{MU8STR("bees"),					{},														MU8STR("minecraft:beehive")},

		{MU8STR("TransferCooldown"),		{MU8STR("Items")},										MU8STR("minecraft:hopper")},
		{MU8STR("SkullOwner"),				{},														MU8STR("minecraft:skull")},

		{MU8STR("Patterns"),				{},														MU8STR("minecraft:banner")},
		{MU8STR("patterns"),				{},														MU8STR("minecraft:banner")},

		{MU8STR("Sherds"),					{},														MU8STR("minecraft:decorated_pot")},
		{MU8STR("sherds"),					{},														MU8STR("minecraft:decorated_pot")},

		{MU8STR("last_interacted_slot"),	{MU8STR("Items")},										MU8STR("minecraft:chiseled_bookshelf")},
		{MU8STR("CookTime"),				{MU8STR("Items")},										MU8STR("minecraft:furnace")},
		{MU8STR("RecordItem"),				{},														MU8STR("minecraft:jukebox")},

		{MU8STR("Book"),					{},														MU8STR("minecraft:lectern")},
		{MU8STR("book"),					{},														MU8STR("minecraft:lectern")},

		{MU8STR("front_text"),				{},														MU8STR("minecraft:sign")},
		{MU8STR("back_text"),				{},														MU8STR("minecraft:sign")},

		{MU8STR("BrewTime"),				{},														MU8STR("minecraft:brewing_stand")},
		{MU8STR("Fuel"),					{},														MU8STR("minecraft:brewing_stand")},

		{MU8STR("LootTable"),				{MU8STR("LootTableSeed")},								MU8STR("minecraft:suspicious_sand")},
		{MU8STR("hit_direction"),			{MU8STR("item")},										MU8STR("minecraft:suspicious_sand")},

		{MU8STR("SpawnData"),				{},														MU8STR("minecraft:spawner")},
		{MU8STR("SpawnPotentials"),			{},														MU8STR("minecraft:spawner")},

		{MU8STR("normal_config"),			{},														MU8STR("minecraft:trial_spawner")},
		{MU8STR("shared_data"),				{},														MU8STR("minecraft:vault")},

		{MU8STR("pool"),					{MU8STR("final_state"),	MU8STR("placement_priority")},	MU8STR("minecraft:jigsaw")},
		{MU8STR("author"),					{MU8STR("metadata"),	MU8STR("showboundingbox")},		MU8STR("minecraft:structure_block")},
		{MU8STR("ExactTeleport"),			{MU8STR("Age")},										MU8STR("minecraft:end_gateway")},
		{MU8STR("Items"),					{},														MU8STR("minecraft:chest")},

		{MU8STR("last_vibration_frequency"),{MU8STR("listener")},									MU8STR("minecraft:sculk_sensor")},
		{MU8STR("warning_level"),			{MU8STR("listener")},									MU8STR("minecraft:sculk_shrieker")},

		{MU8STR("OutputSignal"),			{},														MU8STR("minecraft:comparator")},

		{MU8STR("facing"),					{},														MU8STR("minecraft:piston")},
		{MU8STR("extending"),				{},														MU8STR("minecraft:piston")},
		{MU8STR("x"),						{MU8STR("y"),	MU8STR("z")},							MU8STR("minecraft:piston")},

		{MU8STR("Primary"),					{},														MU8STR("minecraft:beacon")},
		{MU8STR("Secondary"),				{},														MU8STR("minecraft:beacon")},
		{MU8STR("primary_effect"),			{},														MU8STR("minecraft:beacon")},
		{MU8STR("secondary_effect"),		{},														MU8STR("minecraft:beacon")},
	};

	//遍历猜测表，挨个匹配
	for (const auto &[strContains, listAdditional, strIdGuess] : listIdGuess)
	{
		//首先主条件必须匹配，否则跳过
		if (!cpdTileEntity.Contains(strContains))
		{
			continue;
		}

		//附条件匹配，如果附条件为空，则默认匹配成功
		for (const auto &itAdd : listAdditional)
		{
			if (!cpdTileEntity.Contains(itAdd))
			{
				goto Continue_Outer;//重试外层
			}
		}

		//附加条件为空，或不为空且全部匹配，直接插入id并返回成功与否
		cpdTileEntity.PutString(MU8STR("id"), strIdGuess);
		return;

		//附加条件有任意一个不匹配，重试下一个
	Continue_Outer:
		continue;
	}

	//完全无法猜测，插入空值返回
	cpdTileEntity.PutString(MU8STR("id"), MU8STR(""));
	return;
}

//唱片机特殊处理
void ProcessJukebox(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData, const NBT_Type::Int iV7McDataVersion)
{
	cpdV6TileEntityData.PutLong(MU8STR("RecordStartTick"), 0);
	cpdV6TileEntityData.PutLong(MU8STR("TickCount"), nodeV7TagVal.IsLong() ? nodeV7TagVal.GetLong() : 0);
	cpdV6TileEntityData.PutByte(MU8STR("IsPlaying"), 0);
}

void ProcessSingleItem(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7Item = nodeV7Tag.GetCompound();
	auto *pId = cpdV7Item.HasString(MU8STR("id"));
	if (pId == NULL)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV6Item = nodeV6Tag.SetCompound();
	auto &strItemId = cpdV6Item.PutString(MU8STR("id"), std::move(*pId)).first->second.GetString();
	cpdV6Item.PutByte(MU8STR("Count"), CopyOrElse(cpdV7Item.HasInt(MU8STR("count")), 1));

	if (auto *pV7Tag = cpdV7Item.HasCompound(MU8STR("components")); pV7Tag != NULL)
	{
		NBT_Type::Compound cpdV6Tag;
		ProcessComponentsTag(*pV7Tag, strItemId, cpdV6Tag, iV7McDataVersion);
		cpdV6Item.PutCompound(MU8STR("tag"), std::move(cpdV6Tag));
	}

	return;
}

void ProcessSignText(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//所有数据直接插入并保留，仅修改需要的部分
	nodeV6Tag = std::move(nodeV7Tag);//转移所有权

	//1.21.4及以前无需修改
	if (iV7McDataVersion <= MC_1_21_4_MINECRAFT_DATA_VERSION)
	{
		return;
	}

	//1.21.4后修改
	auto &cpdV6 = nodeV6Tag.GetCompound();
	auto *pMessages = cpdV6.HasList(MU8STR("messages"));//编辑V6中的数据

	if (pMessages == NULL)
	{
		return;
	}

	//解析消息，如果不是被引号包围的字符串，那么是1.21.10+，修改为引号包围，同时遍历转义字符串
	for (auto &itV6 : *pMessages)
	{
		NBT_Type::String strJsonTemp{};
		if (!ProcessTextComponent(itV6, strJsonTemp))//nbt格式转化为json数据组件，字符串格式进行转义
		{
			continue;//失败跳过（保持原数据不变），处理下一个
		}

		//成功，重设为json
		itV6.SetString(std::move(strJsonTemp));
	}
}

void ProcessBeaconEffect(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsString())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//查找并映射
	auto &strEffect = nodeV7Tag.GetString();
	NBT_Type::Int iEffect = BeaconEffectMap(strEffect);
	nodeV6Tag.SetInt(iEffect);

	return;
}

void ProcessTileEntity(NBT_Type::Compound &cpdV7TileEntityData, NBT_Type::Compound &cpdV6TileEntityData, const NBT_Type::Int iV7McDataVersion)
{
	FixTileEntityId(cpdV7TileEntityData, iV7McDataVersion);

	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;
	using std::placeholders::_4;

	const static std::unordered_map<NBT_Type::String, MapValFunc_T> mapProccess =
	{
		{ MU8STR("custom_name"),				std::bind(DefaultProcess,	MU8STR("CustomName"),	ProcessCustomNameTag,	_1, _2, _3, _4) },
		{ MU8STR("CustomName"),					std::bind(DefaultProcess,	MU8STR("CustomName"),	ProcessCustomNameTag,	_1, _2, _3, _4) },//高版本仍有麻将忘记改名的遗留物

		{ MU8STR("ticks_since_song_started"),	ProcessJukebox }, //唱片机特殊处理

		{ MU8STR("Items"),						std::bind(DefaultProcess,	MU8STR("Items"),		ProcessItems,			_1, _2, _3, _4) },
		{ MU8STR("patterns"),					std::bind(DefaultProcess,	MU8STR("Patterns"),		ProcessPatterns,		_1, _2, _3, _4) },
		{ MU8STR("profile"),					std::bind(DefaultProcess,	MU8STR("SkullOwner"),	ProcessSkullProfile,	_1, _2, _3, _4) },
		{ MU8STR("flower_pos"),					std::bind(DefaultProcess,	MU8STR("FlowerPos"),	ProcessBlockPos,		_1, _2, _3, _4) },
		{ MU8STR("exit_portal"),				std::bind(DefaultProcess,	MU8STR("ExitPortal"),	ProcessBlockPos,		_1, _2, _3, _4) },
		{ MU8STR("bees"),						std::bind(DefaultProcess,	MU8STR("Bees"),			ProcessBees,			_1, _2, _3, _4) },
		{ MU8STR("item"),						std::bind(DefaultProcess,	MU8STR("item"),			ProcessSingleItem,		_1, _2, _3, _4) },
		{ MU8STR("RecordItem"),					std::bind(DefaultProcess,	MU8STR("RecordItem"),	ProcessSingleItem,		_1, _2, _3, _4) },
		{ MU8STR("Book"),						std::bind(DefaultProcess,	MU8STR("Book"),			ProcessSingleItem,		_1, _2, _3, _4) },

		{ MU8STR("front_text"),					std::bind(DefaultProcess,	MU8STR("front_text"),	ProcessSignText,		_1, _2, _3, _4) },
		{ MU8STR("back_text"),					std::bind(DefaultProcess,	MU8STR("back_text"),	ProcessSignText,		_1, _2, _3, _4) },

		{ MU8STR("primary_effect"),				std::bind(DefaultProcess,	MU8STR("Primary"),		ProcessBeaconEffect,	_1, _2, _3, _4) },
		{ MU8STR("secondary_effect"),			std::bind(DefaultProcess,	MU8STR("Secondary"),	ProcessBeaconEffect,	_1, _2, _3, _4) },
	};

	for (auto &[itV7TagKey, itV7TagVal] : cpdV7TileEntityData)
	{
		//查找是否有匹配的处理过程
		auto itFind = mapProccess.find(itV7TagKey);
		if (itFind == mapProccess.end())
		{
			//不匹配直接移动处理
			cpdV6TileEntityData.Put(itV7TagKey, std::move(itV7TagVal));
			continue;
		}

		//进行处理
		auto &funcProcess = itFind->second;
		funcProcess(itV7TagKey, itV7TagVal, cpdV6TileEntityData, iV7McDataVersion);
	}

	return;
}
