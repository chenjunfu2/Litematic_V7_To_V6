#pragma once

#include "nbt_cpp/NBT_All.hpp"
#include "util/MyAssert.hpp"

#include <vector>

bool FixTileEntityId(NBT_Type::Compound &cpdTileEntity)
{
	if (cpdTileEntity.ContainsString(MU8STR("id")))
	{
		return true;
	}

	const auto *pId = cpdTileEntity.HasString(MU8STR("Id"));
	if (pId != NULL)
	{
		cpdTileEntity.PutString(MU8STR("id"), *pId);//转化为小写id
		return true;
	}

	//找不到Id或者Id类型不是字符串，通过其它项猜测类型
	struct GuessNode
	{
		NBT_Type::String strContains;//主条件
		std::vector<NBT_Type::String> listAdditional;//附加条件
		NBT_Type::String strIdGuess;
	};

	const std::vector<GuessNode> listIdGuess =
	{
		{MU8STR("Bees"), {}, MU8STR("minecraft:beehive")},
		{MU8STR("bees"), {}, MU8STR("minecraft:beehive")},

		{MU8STR("TransferCooldown"), {MU8STR("Items")}, MU8STR("minecraft:hopper")},
		{MU8STR("SkullOwner"), {}, MU8STR("minecraft:skull")},

		{MU8STR("Patterns"), {}, MU8STR("minecraft:banner")},
		{MU8STR("patterns"), {}, MU8STR("minecraft:banner")},

		{MU8STR("Sherds"), {}, MU8STR("minecraft:decorated_pot")},
		{MU8STR("sherds"), {}, MU8STR("minecraft:decorated_pot")},

		{MU8STR("last_interacted_slot"), {MU8STR("Items")}, MU8STR("minecraft:chiseled_bookshelf")},
		{MU8STR("CookTime"), {MU8STR("Items")}, MU8STR("minecraft:furnace")},
		{MU8STR("RecordItem"), {}, MU8STR("minecraft:jukebox")},

		{MU8STR("Book"), {}, MU8STR("minecraft:lectern")},
		{MU8STR("book"), {}, MU8STR("minecraft:lectern")},

		{MU8STR("front_text"), {}, MU8STR("minecraft:sign")},

		{MU8STR("BrewTime"), {}, MU8STR("minecraft:brewing_stand")},
		{MU8STR("Fuel"), {}, MU8STR("minecraft:brewing_stand")},

		{MU8STR("LootTable"), {MU8STR("LootTableSeed")}, MU8STR("minecraft:suspicious_sand")},
		{MU8STR("hit_direction"), {MU8STR("item")}, MU8STR("minecraft:suspicious_sand")},

		{MU8STR("SpawnData"), {}, MU8STR("minecraft:spawner")},
		{MU8STR("SpawnPotentials"), {}, MU8STR("minecraft:spawner")},

		{MU8STR("normal_config"), {}, MU8STR("minecraft:trial_spawner")},
		{MU8STR("shared_data"), {}, MU8STR("minecraft:vault")},

		{MU8STR("pool"), {MU8STR("final_state"),MU8STR("placement_priority")}, MU8STR("minecraft:jigsaw")},
		{MU8STR("author"), {MU8STR("metadata"),MU8STR("showboundingbox")}, MU8STR("minecraft:structure_block")},
		{MU8STR("ExactTeleport"), {MU8STR("Age")}, MU8STR("minecraft:end_gateway")},
		{MU8STR("Items"), {}, MU8STR("minecraft:chest")},

		{MU8STR("last_vibration_frequency"), {MU8STR("listener")}, MU8STR("minecraft:sculk_sensor")},
		{MU8STR("warning_level"), {MU8STR("listener")}, MU8STR("minecraft:sculk_shrieker")},

		{MU8STR("OutputSignal"), {}, MU8STR("minecraft:comparator")},

		{MU8STR("facing"), {}, MU8STR("minecraft:piston")},
		{MU8STR("extending"), {}, MU8STR("minecraft:piston")},
		{MU8STR("x"), {MU8STR("y"),MU8STR("z")}, MU8STR("minecraft:piston")},
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
		return true;

		//附加条件有任意一个不匹配，重试下一个
	Continue_Outer:
		continue;
	}

	//完全无法猜测，插入空值返回
	cpdTileEntity.PutString(MU8STR("id"), MU8STR(""));
	return true;
}


bool ProcessItems(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{







}

