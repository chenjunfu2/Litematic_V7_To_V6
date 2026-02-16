#pragma once

#include "nbt_cpp/NBT_All.hpp"

#include <vector>
#include <unordered_map>

void FixTileEntityId(NBT_Type::Compound &cpdTileEntity)
{
	if (cpdTileEntity.ContainsString(MU8STR("id")))
	{
		return;
	}

	const auto *pId = cpdTileEntity.HasString(MU8STR("Id"));
	if (pId != NULL)
	{
		cpdTileEntity.PutString(MU8STR("id"), *pId);//转化为小写id
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

		{MU8STR("BrewTime"),				{},														MU8STR("minecraft:brewing_stand")},
		{MU8STR("Fuel"),					{},														MU8STR("minecraft:brewing_stand")},

		{MU8STR("LootTable"),				{MU8STR("LootTableSeed")},								MU8STR("minecraft:suspicious_sand")},
		{MU8STR("hit_direction"),			{MU8STR("item")},										MU8STR("minecraft:suspicious_sand")},

		{MU8STR("SpawnData"),				{},														MU8STR("minecraft:spawner")},
		{MU8STR("SpawnPotentials"),			{},														MU8STR("minecraft:spawner")},

		{MU8STR("normal_config"),			{},														MU8STR("minecraft:trial_spawner")},
		{MU8STR("shared_data"),				{},														MU8STR("minecraft:vault")},

		{MU8STR("pool"),					{MU8STR("final_state"), MU8STR("placement_priority")},	MU8STR("minecraft:jigsaw")},
		{MU8STR("author"),					{MU8STR("metadata"), MU8STR("showboundingbox")},		MU8STR("minecraft:structure_block")},
		{MU8STR("ExactTeleport"),			{MU8STR("Age")},										MU8STR("minecraft:end_gateway")},
		{MU8STR("Items"),					{},														MU8STR("minecraft:chest")},

		{MU8STR("last_vibration_frequency"),{MU8STR("listener")},									MU8STR("minecraft:sculk_sensor")},
		{MU8STR("warning_level"),			{MU8STR("listener")},									MU8STR("minecraft:sculk_shrieker")},

		{MU8STR("OutputSignal"),			{},														MU8STR("minecraft:comparator")},

		{MU8STR("facing"),					{},														MU8STR("minecraft:piston")},
		{MU8STR("extending"),				{},														MU8STR("minecraft:piston")},
		{MU8STR("x"),						{MU8STR("y"), MU8STR("z")},								MU8STR("minecraft:piston")},
	};

	//遍历猜测表，挨个匹配
	for (const auto &[strContains, listAdditional, strIdGuess] : listIdGuess)
	{
		//首先主条件必须匹配，否则跳过
		if (!cpdTileEntity.Contains(strContains))
		{
			continue;
		}

		//主条件匹配，如果附条件为空，则跳过
		if (listAdditional.empty())
		{
			continue;
		}

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


void ProcessItems(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessPatterns(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsList())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	NBT_Type::List listV7 = nodeV7Tag.GetList();
	NBT_Type::List listV6 = nodeV6Tag.SetList();

	const static NBT_Type::Int iDefaultColor = 0;
	const static std::unordered_map<NBT_Type::String, NBT_Type::Int> mapColor =
	{
		{MU8STR("white"), 0},		{MU8STR("orange"), 1},		{MU8STR("magenta"), 2},		{MU8STR("light_blue"), 3},
		{MU8STR("yellow"), 4},		{MU8STR("lime"), 5},		{MU8STR("pink"), 6},		{MU8STR("gray"), 7},
		{MU8STR("light_gray"), 8},	{MU8STR("cyan"), 9},		{MU8STR("purple"), 10},		{MU8STR("blue"), 11},
		{MU8STR("brown"), 12},		{MU8STR("green"), 13},		{MU8STR("red"), 14},		{MU8STR("black"), 15}
	};

	const static NBT_Type::String strDefaultPattern = MU8STR("b");
	const static std::unordered_map<NBT_Type::String, NBT_Type::String> mapPattern =
	{
		{ MU8STR("minecraft:base"),						MU8STR("b") },
		{ MU8STR("minecraft:square_bottom_left"),		MU8STR("bl") },
		{ MU8STR("minecraft:square_bottom_right"),		MU8STR("br") },
		{ MU8STR("minecraft:square_top_left"),			MU8STR("tl") },
		{ MU8STR("minecraft:square_top_right"),			MU8STR("tr") },
		{ MU8STR("minecraft:stripe_bottom"),			MU8STR("bs") },
		{ MU8STR("minecraft:stripe_top"),				MU8STR("ts") },
		{ MU8STR("minecraft:stripe_left"),				MU8STR("ls") },
		{ MU8STR("minecraft:stripe_right"),				MU8STR("rs") },
		{ MU8STR("minecraft:stripe_center"),			MU8STR("cs") },
		{ MU8STR("minecraft:stripe_middle"),			MU8STR("ms") },
		{ MU8STR("minecraft:stripe_downright"),			MU8STR("drs") },
		{ MU8STR("minecraft:stripe_downleft"),			MU8STR("dls") },
		{ MU8STR("minecraft:small_stripes"),			MU8STR("ss") },
		{ MU8STR("minecraft:cross"),					MU8STR("cr") },
		{ MU8STR("minecraft:straight_cross"),			MU8STR("sc") },
		{ MU8STR("minecraft:triangle_bottom"),			MU8STR("bt") },
		{ MU8STR("minecraft:triangle_top"),				MU8STR("tt") },
		{ MU8STR("minecraft:triangles_bottom"),			MU8STR("bts") },
		{ MU8STR("minecraft:triangles_top"),			MU8STR("tts") },
		{ MU8STR("minecraft:diagonal_left"),			MU8STR("ld") },
		{ MU8STR("minecraft:diagonal_up_right"),		MU8STR("rd") },
		{ MU8STR("minecraft:diagonal_up_left"),			MU8STR("lud") },
		{ MU8STR("minecraft:diagonal_right"),			MU8STR("rud") },
		{ MU8STR("minecraft:circle"),					MU8STR("mc") },
		{ MU8STR("minecraft:rhombus"),					MU8STR("mr") },
		{ MU8STR("minecraft:half_vertical"),			MU8STR("vh") },
		{ MU8STR("minecraft:half_horizontal"),			MU8STR("hh") },
		{ MU8STR("minecraft:half_vertical_right"),		MU8STR("vhr") },
		{ MU8STR("minecraft:half_horizontal_bottom"),	MU8STR("hhb") },
		{ MU8STR("minecraft:border"),					MU8STR("bo") },
		{ MU8STR("minecraft:curly_border"),				MU8STR("cbo") },
		{ MU8STR("minecraft:gradient"),					MU8STR("gra") },
		{ MU8STR("minecraft:gradient_up"),				MU8STR("gru") },
		{ MU8STR("minecraft:bricks"),					MU8STR("bri") },
		{ MU8STR("minecraft:globe"),					MU8STR("glb") },
		{ MU8STR("minecraft:creeper"),					MU8STR("cre") },
		{ MU8STR("minecraft:skull"),					MU8STR("sku") },
		{ MU8STR("minecraft:flower"),					MU8STR("flo") },
		{ MU8STR("minecraft:mojang"),					MU8STR("moj") },
		{ MU8STR("minecraft:piglin"),					MU8STR("pig") },
	};

	for (auto &itEntry : listV7)
	{
		if (!itEntry.IsCompound())
		{
			continue;
		}

		auto &cpdV7Entry = itEntry.GetCompound();
		auto &cpdV6Entry = listV6.AddBackCompound({}).first->GetCompound();

		//查找并映射颜色
		NBT_Type::Int iColor = iDefaultColor;

		auto *pstrColor = cpdV7Entry.HasString(MU8STR("color"));
		if (pstrColor != NULL)
		{
			auto itFind = mapColor.find(*pstrColor);
			if (itFind != mapColor.end())
			{
				iColor = itFind->second;
			}
		}

		//插入
		cpdV6Entry.PutInt(MU8STR("Color"), iColor);

		//查找并映射图样
		NBT_Type::String strPattern = strDefaultPattern;

		auto *pstrPattern = cpdV7Entry.HasString(MU8STR("pattern"));
		if (pstrPattern != NULL)
		{
			auto itFind = mapPattern.find(*pstrPattern);
			if (itFind != mapPattern.end())
			{
				strPattern = itFind->second;
			}
		}

		//插入
		cpdV6Entry.PutString(MU8STR("Pattern"), strPattern);
	}

	return;
}

void ProcessSkullProfile(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessFlowerPos(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessBees(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessSingleItem(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessRecordItem(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessBook(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessCustomName(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

