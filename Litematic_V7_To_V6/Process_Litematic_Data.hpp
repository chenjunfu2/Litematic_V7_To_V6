#pragma once

#include "nbt_cpp/NBT_All.hpp"

#include <vector>
#include <unordered_map>
#include <functional>
#include <utility>

//前向声明
void ProcessEntity(NBT_Type::Compound &cpdV7EntityData, NBT_Type::Compound &cpdV6EntityData);
void ProcessTileEntity(NBT_Type::Compound &cpdV7TileEntityData, NBT_Type::Compound &cpdV6TileEntityData);
void ProcessBlockPos(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag,
	const NBT_Type::String &strX = MU8STR("X"),
	const NBT_Type::String &strY = MU8STR("Y"),
	const NBT_Type::String &strZ = MU8STR("Z"));

template<typename T, typename V>
requires(std::is_same_v<std::decay_t<T>, std::decay_t<V>> || std::is_constructible_v<T, V>)
T CopyOrElse(T *p, V &&d)
{
	return p != NULL ? *p : std::forward<V>(d);
}

template<typename T, typename V>
requires(std::is_same_v<std::decay_t<T>, std::decay_t<V>> || std::is_constructible_v<T, V>)
T MoveOrElse(T *p, V &&d)
{
	return p != NULL ? std::move(*p) : std::forward<V>(d);
}

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

NBT_Type::String AttributeNameMap(const NBT_Type::String& strAttrName)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::String> mapAttributeName =
	{
		{ MU8STR("minecraft:armor"),							MU8STR("minecraft:generic.armor") },
		{ MU8STR("minecraft:armor_toughness"),					MU8STR("minecraft:generic.armor_toughness") },
		{ MU8STR("minecraft:attack_damage"),					MU8STR("minecraft:generic.attack_damage") },
		{ MU8STR("minecraft:attack_knockback"),					MU8STR("minecraft:generic.attack_knockback") },
		{ MU8STR("minecraft:attack_speed"),						MU8STR("minecraft:generic.attack_speed") },
		{ MU8STR("minecraft:flying_speed"),						MU8STR("minecraft:generic.flying_speed") },
		{ MU8STR("minecraft:follow_range"),						MU8STR("minecraft:generic.follow_range") },
		{ MU8STR("minecraft:jump_strength"),					MU8STR("minecraft:horse.jump_strength") },
		{ MU8STR("minecraft:knockback_resistance"),				MU8STR("minecraft:generic.knockback_resistance") },
		{ MU8STR("minecraft:luck"),								MU8STR("minecraft:generic.luck") },
		{ MU8STR("minecraft:max_absorption"),					MU8STR("minecraft:generic.max_absorption") },
		{ MU8STR("minecraft:max_health"),						MU8STR("minecraft:generic.max_health") },
		{ MU8STR("minecraft:movement_speed"),					MU8STR("minecraft:generic.movement_speed") },
		{ MU8STR("minecraft:spawn_reinforcements"),				MU8STR("minecraft:zombie.spawn_reinforcements") },
		{ MU8STR("minecraft:block_break_speed"),				MU8STR("minecraft:player.block_break_speed") },
		{ MU8STR("minecraft:block_interaction_range"),			MU8STR("minecraft:player.block_interaction_range") },
		{ MU8STR("minecraft:burning_time"),						MU8STR("minecraft:generic.burning_time") },
		{ MU8STR("minecraft:explosion_knockback_resistance"),	MU8STR("minecraft:generic.explosion_knockback_resistance") },
		{ MU8STR("minecraft:entity_interaction_range"),			MU8STR("minecraft:player.entity_interaction_range") },
		{ MU8STR("minecraft:fall_damage_multiplier"),			MU8STR("minecraft:generic.fall_damage_multiplier") },
		{ MU8STR("minecraft:gravity"),							MU8STR("minecraft:generic.gravity") },
		{ MU8STR("minecraft:mining_efficiency"),				MU8STR("minecraft:player.mining_efficiency") },
		{ MU8STR("minecraft:movement_efficiency"),				MU8STR("minecraft:generic.movement_efficiency") },
		{ MU8STR("minecraft:oxygen_bonus"),						MU8STR("minecraft:generic.oxygen_bonus") },
		{ MU8STR("minecraft:safe_fall_distance"),				MU8STR("minecraft:generic.safe_fall_distance") },
		{ MU8STR("minecraft:scale"),							MU8STR("minecraft:generic.scale") },
		{ MU8STR("minecraft:sneaking_speed"),					MU8STR("minecraft:player.sneaking_speed") },
		{ MU8STR("minecraft:step_height"),						MU8STR("minecraft:generic.step_height") },
		{ MU8STR("minecraft:submerged_mining_speed"),			MU8STR("minecraft:player.submerged_mining_speed") },
		{ MU8STR("minecraft:sweeping_damage_ratio"),			MU8STR("minecraft:player.sweeping_damage_ratio") },
		{ MU8STR("minecraft:water_movement_efficiency"),		MU8STR("minecraft:generic.water_movement_efficiency") },
	};

	auto itFind = mapAttributeName.find(strAttrName);
	return itFind != mapAttributeName.end() ? itFind->second : strAttrName;
}

void ProcessAttributeModifiers(NBT_Type::List &listV7Tag, NBT_Type::List &listV6Tag)
{
	for (auto &itV7Entry : listV7Tag)
	{
		if (!itV7Entry.IsCompound())
		{
			continue;
		}

		auto &cpdV7Entry = itV7Entry.GetCompound();
		NBT_Type::Compound cpdV6Entry;

		if (cpdV7Entry.Contains(MU8STR("type")))
		{
			auto *pType = cpdV7Entry.HasString(MU8STR("type"));
			cpdV6Entry.PutString(MU8STR("Name"), AttributeNameMap(pType != NULL ? *pType : MU8STR("")));
			cpdV6Entry.PutDouble(MU8STR("Base"), CopyOrElse(cpdV7Entry.HasDouble(MU8STR("amount")), 0.0));

		}
		else
		{
			auto *pId = cpdV7Entry.HasString(MU8STR("id"));
			if (pId != NULL && *pId == MU8STRV("minecraft:random_spawn_bonus"))
			{
				cpdV6Entry.PutString(MU8STR("Name"), MU8STR("Random spawn bonus"));
			}
			else
			{
				cpdV6Entry.PutString(MU8STR("Name"), MU8STR(""));
			}
			cpdV6Entry.PutDouble(MU8STR("Amount"), CopyOrElse(cpdV7Entry.HasDouble(MU8STR("amount")), 0.0));
		}

		std::unordered_map<NBT_Type::String, NBT_Type::Int> mapOperation =
		{
			{ MU8STR("add_value"),				0 },
			{ MU8STR("add_multiplied_base"),	1 },
			{ MU8STR("add_multiplied_total"),	2 },
		};

		NBT_Type::Int iOperation = 0;//default
		if (auto *pOperation = cpdV7Entry.HasString(MU8STR("operation")); pOperation != NULL)
		{
			if (auto itFind = mapOperation.find(*pOperation); itFind != mapOperation.end())
			{
				iOperation = itFind->second;
			}
		}
		cpdV6Entry.PutInt(MU8STR("Operation"), iOperation);

		if (auto *pUUID = cpdV7Entry.Has(MU8STR("UUID")); pUUID != NULL)
		{
			cpdV6Entry.Put(MU8STR("UUID"), *pUUID);
		}

		listV6Tag.AddBack(std::move(cpdV6Entry));
	}
}

void ProcessAttributes(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsList())
	{
		if (!nodeV7Tag.IsCompound())//如果不是列表也不是集合，那么跳过
		{
			nodeV6Tag = std::move(nodeV7Tag);
			return;
		}

		auto &cpdV7 = nodeV7Tag.GetCompound();
		auto &listV6 = nodeV6Tag.SetList();

		for (auto &[strV7Key, nodeV7Val] : cpdV7)
		{
			if (strV7Key == MU8STRV("modifiers") && nodeV7Val.IsList())
			{
				ProcessAttributeModifiers(nodeV7Val.GetList(), listV6);
				return;
			}
		}
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

		if (cpdV7Entry.Contains(MU8STR("type")))
		{
			auto *pType = cpdV7Entry.HasString(MU8STR("type"));
			cpdV6Entry.PutString(MU8STR("Name"), AttributeNameMap(pType != NULL ? *pType : MU8STR("")));
			cpdV6Entry.PutDouble(MU8STR("Base"), CopyOrElse(cpdV7Entry.HasDouble(MU8STR("amount")), 0.0));
		}
		else
		{
			auto *pId = cpdV7Entry.HasString(MU8STR("id"));
			cpdV6Entry.PutString(MU8STR("Name"), AttributeNameMap(pId != NULL ? *pId : MU8STR("")));
			cpdV6Entry.PutDouble(MU8STR("Base"), CopyOrElse(cpdV7Entry.HasDouble(MU8STR("base")), 0.0));
		}


		auto *pModifiers = cpdV7Entry.HasList(MU8STR("modifiers"));
		if (pModifiers != NULL)
		{
			NBT_Type::List listV6Modifiers;
			ProcessAttributeModifiers(*pModifiers, listV6Modifiers);
			if (!listV6Modifiers.Empty())
			{
				cpdV6Entry.PutList(MU8STR("Modifiers"), std::move(listV6Modifiers));
			}
		}

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
}

void ProcessEnchantments(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto *pLevels = GetCompound(nodeV7Tag).HasCompound(MU8STR("levels"));
	if (pLevels == NULL)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &listV6 = nodeV6Tag.SetList();
	for (auto &[strV7Key, nodeV7Val] : *pLevels)//集合相当于列表，每个key是附魔，val是值
	{
		NBT_Type::Compound cpdV6Entry;

		//插入名称
		cpdV6Entry.PutString(MU8STR("id"), strV7Key);

		//插入等级
		//新版本为Int，检查范围
		auto iLevel = nodeV7Val.IsInt() ? nodeV7Val.GetInt() : 1;
		iLevel = iLevel > NBT_Type::Short_Max ? NBT_Type::Short_Max : iLevel;
		iLevel = iLevel < NBT_Type::Short_Min ? NBT_Type::Short_Min : iLevel;
		//转换为老版本Short
		cpdV6Entry.PutShort(MU8STR("lvl"), (NBT_Type::Short)iLevel);

		//集合放入旧版列表
		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
}

void ProcessEntityTag(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	ProcessEntity(nodeV7Tag.GetCompound(), nodeV6Tag.SetCompound());
	return;
}

void ProcessFireworkExplosion(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	static const std::unordered_map<NBT_Type::String, NBT_Type::Byte> mapShape =
	{
		{ MU8STR("small_ball"),	0 },
		{ MU8STR("large_ball"),	1 },
		{ MU8STR("star"),		2 },
		{ MU8STR("creeper"),	3 },
		{ MU8STR("burst"),		4 },
	};

	auto &cpdV7Tag = nodeV7Tag.GetCompound();
	auto &cpdV6Tag = nodeV6Tag.SetCompound();

	auto *pShape = cpdV7Tag.HasString(MU8STR("shape"));
	if (pShape != NULL)
	{
		NBT_Type::Byte bType = 0;//default

		auto itFind = mapShape.find(*pShape);
		if (itFind != mapShape.end())
		{
			bType = itFind->second;
		}

		cpdV6Tag.PutByte(MU8STR("Type"), bType);
	}

	auto *pColors = cpdV7Tag.HasIntArray(MU8STR("colors"));
	if (pColors != NULL)
	{
		cpdV6Tag.PutIntArray(MU8STR("Colors"), *pColors);
	}

	auto *pFadeColors = cpdV7Tag.HasIntArray(MU8STR("fade_colors"));
	if (pFadeColors != NULL)
	{
		cpdV6Tag.PutIntArray(MU8STR("FadeColors"), *pFadeColors);
	}

	auto *pHasTrail = cpdV7Tag.HasByte(MU8STR("has_trail"));
	if (pHasTrail != NULL)
	{
		cpdV6Tag.PutByte(MU8STR("Trail"), *pHasTrail);
	}

	auto *pHasTwinkle = cpdV7Tag.HasByte(MU8STR("has_twinkle"));
	if (pHasTwinkle != NULL)
	{
		cpdV6Tag.PutByte(MU8STR("Flicker"), *pHasTwinkle);
	}

	return;
}

void ProcessFireworks(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7 = nodeV7Tag.GetCompound();
	auto &cpdV6 = nodeV6Tag.SetCompound();

	cpdV6.PutByte(MU8STR("Flight"), CopyOrElse(cpdV7.HasByte(MU8STR("flight_duration")), 1));

	auto *pExplosions = cpdV7.HasList(MU8STR("explosions"));
	if (pExplosions != NULL)
	{
		auto &listV7 = *pExplosions;
		NBT_Type::List listV6;

		for (auto &itV7Entry : listV7)
		{
			if (!itV7Entry.IsCompound())
			{
				continue;
			}

			NBT_Node nodeV6Explosion;
			ProcessFireworkExplosion(itV7Entry, nodeV6Explosion);
			listV6.AddBack(std::move(nodeV6Explosion));
		}

		cpdV6.PutList(MU8STR("Explosions"), std::move(listV6));
	}

	return;
}

NBT_Type::Byte DecorationTypeMap(const NBT_Type::String &strType)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::Byte> mapDecorationType =
	{
		{ MU8STR("minecraft:player"),				 0 },
		{ MU8STR("minecraft:frame"),				 1 },
		{ MU8STR("minecraft:red_marker"),			 2 },
		{ MU8STR("minecraft:blue_marker"),			 3 },
		{ MU8STR("minecraft:target_x"),				 4 },
		{ MU8STR("minecraft:target_point"),			 5 },
		{ MU8STR("minecraft:player_off_map"),		 6 },
		{ MU8STR("minecraft:player_off_limits"),	 7 },
		{ MU8STR("minecraft:mansion"),				 8 },
		{ MU8STR("minecraft:monument"),				 9 },
		{ MU8STR("minecraft:banner_white"),			10 },
		{ MU8STR("minecraft:banner_orange"),		11 },
		{ MU8STR("minecraft:banner_magenta"),		12 },
		{ MU8STR("minecraft:banner_light_blue"),	13 },
		{ MU8STR("minecraft:banner_yellow"),		14 },
		{ MU8STR("minecraft:banner_lime"),			15 },
		{ MU8STR("minecraft:banner_pink"),			16 },
		{ MU8STR("minecraft:banner_gray"),			17 },
		{ MU8STR("minecraft:banner_light_gray"),	18 },
		{ MU8STR("minecraft:banner_cyan"),			19 },
		{ MU8STR("minecraft:banner_purple"),		20 },
		{ MU8STR("minecraft:banner_blue"),			21 },
		{ MU8STR("minecraft:banner_brown"),			22 },
		{ MU8STR("minecraft:banner_green"),			23 },
		{ MU8STR("minecraft:banner_red"),			24 },
		{ MU8STR("minecraft:banner_black"),			25 },
		{ MU8STR("minecraft:red_x"),				26 },
		{ MU8STR("minecraft:village_desert"),		27 },
		{ MU8STR("minecraft:village_plains"),		28 },
		{ MU8STR("minecraft:village_savanna"),		29 },
		{ MU8STR("minecraft:village_snowy"),		30 },
		{ MU8STR("minecraft:village_taiga"),		31 },
		{ MU8STR("minecraft:jungle_temple"),		32 },
		{ MU8STR("minecraft:swamp_hut"),			33 },
	};

	auto itFind = mapDecorationType.find(strType);
	return itFind != mapDecorationType.end() ? itFind->second : 0;
}

void ProcessMapDecorations(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7 = nodeV7Tag.GetCompound();
	auto &listV6 = nodeV6Tag.SetList();

	for (auto &[strV7Key, nodeV7Val] : cpdV7)
	{
		if (!nodeV7Val.IsCompound())
		{
			continue;
		}

		auto &cpdV7Entry = nodeV7Val.GetCompound();
		NBT_Type::Compound cpdV6Entry;

		cpdV6Entry.PutString(MU8STR("id"), strV7Key);
		cpdV6Entry.PutDouble(MU8STR("x"), CopyOrElse(cpdV7Entry.HasDouble(MU8STR("x")), 0.0));
		cpdV6Entry.PutDouble(MU8STR("z"), CopyOrElse(cpdV7Entry.HasDouble(MU8STR("z")), 0.0));
		cpdV6Entry.PutDouble(MU8STR("rot"), (NBT_Type::Double)CopyOrElse(cpdV7Entry.HasFloat(MU8STR("rotation")), 0.0f));

		NBT_Type::Byte bType = 0;//default
		if (auto *pType = cpdV7Entry.HasString(MU8STR("type")); pType != NULL)
		{
			bType = DecorationTypeMap(*pType);
		}
		cpdV6Entry.PutByte(MU8STR("type"), bType);

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
}

void ProcessUnbreakable(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//子组件在低版本不存在，直接忽略，注意高版本中，
	//只要挂载了这个标签，也就是这个函数被调用则生效Unbreakable，
	//所以直接设置nodeV6Tag为boolean=true（也就是byte=1）
	nodeV6Tag.SetByte(1);

	return;
}

void CustomDataProcess(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TileEntityData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	//事实上，CustomData内部存储的key val在低版本是直接放在外面的，所以，解包出来的值直接合并即可
	cpdV6TileEntityData.Merge(std::move(nodeV7TagVal.GetCompound()));
	return;
}

void LodestoneTrackerProcess(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TileEntityData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto &cpdV7 = nodeV7TagVal.GetCompound();

	//与CustomData一致，数据存储在外面，但是名称有变化
	if (auto *pTracked = cpdV7.HasByte(MU8STR("tracked")); pTracked != NULL)
	{
		cpdV6TileEntityData.PutByte(MU8STR("LodestoneTracked"), std::move(*pTracked));
	}

	if (auto *pTarget = cpdV7.HasCompound(MU8STR("target")); pTarget != NULL)
	{
		if (auto *pDimension = pTarget->HasString(MU8STR("dimension")); pDimension != NULL)
		{
			cpdV6TileEntityData.PutString(MU8STR("LodestoneDimension"), std::move(*pDimension));
		}

		if (auto *pPos = pTarget->Has(MU8STR("pos")); pPos != NULL)
		{
			NBT_Node nodeV6Pos;
			ProcessBlockPosDefault(*pPos, nodeV6Pos);
			cpdV6TileEntityData.Put(MU8STR("LodestonePos"), std::move(nodeV6Pos));
		}
	}

	return;
}

void PotionContentsProcess(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TileEntityData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto &cpdV7 = nodeV7TagVal.GetCompound();
	if (auto *pPotion = cpdV7.HasString(MU8STR("potion")); pPotion != NULL)
	{
		cpdV6TileEntityData.PutString(MU8STR("Potion"), std::move(*pPotion));
	}
	if (auto *pCustomColor = cpdV7.HasInt(MU8STR("custom_color")); pCustomColor != NULL)
	{
		cpdV6TileEntityData.PutInt(MU8STR("CustomPotionColor"), std::move(*pCustomColor));
	}
	if (auto *pCustomEffects = cpdV7.HasList(MU8STR("custom_effects")); pCustomEffects != NULL)
	{
		cpdV6TileEntityData.PutList(MU8STR("custom_potion_effects"), std::move(*pCustomEffects));
	}

	return;
}

void BlockEntityDataProcess(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TileEntityData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	//直接转换到根
	ProcessTileEntity(nodeV7TagVal.GetCompound(), cpdV6TileEntityData);
	return;
}

void BucketEntityDataProcess(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TileEntityData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	//直接转换到根
	ProcessEntity(nodeV7TagVal.GetCompound(), cpdV6TileEntityData);
	return;
}

void ProcessDyedColor(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto *pRGB = GetCompound(nodeV7Tag).Has(MU8STR("rgb"));
	//要么是Int，要么是列表，否则失败
	bool bIsIntRGB = pRGB->IsInt();
	bool bIsListRGB = pRGB->IsList();
	if (pRGB == NULL || (!bIsIntRGB && !bIsListRGB))
	{
		nodeV6Tag.SetInt(16777215);//default
		return;
	}

	if (bIsIntRGB)
	{
		nodeV6Tag.SetInt(pRGB->GetInt());
		return;
	}
	else if (bIsListRGB)
	{
		auto &listRGB = pRGB->GetList();
		if (listRGB.GetTag() != NBT_TAG::Float || listRGB.Size() != 3)
		{
			nodeV6Tag.SetInt(0xFF'FF'FF'FF);//default
			return;
		}

		auto fR = listRGB.GetFloat(0);
		auto fG = listRGB.GetFloat(1);
		auto fB = listRGB.GetFloat(2);

		NBT_Type::Byte bR = 255.0 * fR;
		NBT_Type::Byte bG = 255.0 * fG;
		NBT_Type::Byte bB = 255.0 * fB;

		NBT_Type::Int iRGB =
			(NBT_Type::Int)255 << 24 |
			(NBT_Type::Int)bR  << 16 |
			(NBT_Type::Int)bG  <<  8 |
			(NBT_Type::Int)bR  <<  0;

		nodeV6Tag.SetInt(iRGB);
	}

	return;
}

void ProcessCustomNameTag(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	//与ProcessItemName一样，放弃处理

	nodeV6Tag = std::move(nodeV7Tag);

	return;
}

void ProcessItemName(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	//旧版中，Name标签只会是Json字符串
	//新版可能是Compound、List或SNBT或Json
	//根据实际情况，极其复杂，选择放弃处理，直接移动

	nodeV6Tag = std::move(nodeV7Tag);

	return;
}


//实际转换
using TagProcessFunc_T = std::function<void(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)>;
//用于Map值
using MapValFunc_T = std::function<void(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)>;

//通用处理
void DefaultProcess(const NBT_Type::String &strNewKey, TagProcessFunc_T funcTagProcess, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)
{
	NBT_Node nodeV6TagVal;
	funcTagProcess(nodeV7TagVal, nodeV6TagVal);
	cpdV6TileEntityData.Put(strNewKey, std::move(nodeV6TagVal));
};

void RenameProcess(const NBT_Type::String &strNewKey, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData)
{
	cpdV6TileEntityData.Put(strNewKey, std::move(nodeV7TagVal));
}

void ProcessComponentsTag(NBT_Type::Compound &cpdV7Tag, const NBT_Type::String &strItemId, NBT_Type::Compound &cpdV6Tag)
{
	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;

	enum class UseTagType
	{
		V6Tag,
		BlockEntityTag,
		DisplayTag,
	};

	struct MapVal_T
	{
		bool bUseItemId;
		UseTagType enUseTagType;
		MapValFunc_T funcProcess;
	};

	const static std::unordered_map<NBT_Type::String, MapVal_T> mapProccess =
	{
		{ MU8STR("minecraft:block_state"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("BlockStateTag"),		_1, _2, _3) } },
		{ MU8STR("minecraft:instrument"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("instrument"),			_1, _2, _3) } },
		{ MU8STR("minecraft:map_id"),					{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("map"),					_1, _2, _3) } },
		{ MU8STR("minecraft:recipes"),					{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("Recipes"),				_1, _2, _3) } },
		{ MU8STR("minecraft:suspicious_stew_effects"),	{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("effects"),				_1, _2, _3) } },
		{ MU8STR("minecraft:trim"),						{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("Trim"),					_1, _2, _3) } },

		{ MU8STR("minecraft:can_break"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("CanDestroy"),			_1, _2, _3) } },
		{ MU8STR("minecraft:can_place_on"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("CanPlaceOn"),			_1, _2, _3) } },
		{ MU8STR("minecraft:custom_model_data"),		{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("CustomModelData"),		_1, _2, _3) } },
		{ MU8STR("minecraft:damage"),					{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("Damage"),				_1, _2, _3) } },
		{ MU8STR("minecraft:debug_stick_state"),		{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("DebugProperty"),		_1, _2, _3) } },
		{ MU8STR("minecraft:note_block_sound"),			{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("note_block_sound"),		_1, _2, _3) } },
		{ MU8STR("minecraft:repair_cost"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("RepairCost"),			_1, _2, _3) } },

		{ MU8STR("minecraft:attribute_modifiers"),		{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("AttributeModifiers"),	ProcessAttributes,				_1, _2, _3) } },
		{ MU8STR("minecraft:bundle_contents"),			{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Items"),				ProcessItems,					_1, _2, _3) } },
		{ MU8STR("minecraft:enchantments"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Enchantments"),			ProcessEnchantments,			_1, _2, _3) } },
		{ MU8STR("minecraft:entity_data"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("EntityTag"),			ProcessEntityTag,				_1, _2, _3) } },
		{ MU8STR("minecraft:stored_enchantments"),		{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("StoredEnchantments"),	ProcessEnchantments,			_1, _2, _3) } },
		{ MU8STR("minecraft:fireworks"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Fireworks"),			ProcessFireworks,				_1, _2, _3) } },
		{ MU8STR("minecraft:firework_explosion"),		{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Explosion"),			ProcessFireworkExplosion,		_1, _2, _3) } },
		{ MU8STR("minecraft:map_decorations"),			{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Decorations"),			ProcessMapDecorations,			_1, _2, _3) } },
		{ MU8STR("minecraft:profile"),					{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("SkullOwner"),			ProcessSkullProfile,			_1, _2, _3) } },
		{ MU8STR("minecraft:unbreakable"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Unbreakable"),			ProcessUnbreakable,				_1, _2, _3) } },

		{ MU8STR("minecraft:custom_data"),				{ false,	UseTagType::V6Tag,			CustomDataProcess } },
		{ MU8STR("minecraft:lodestone_tracker"),		{ false,	UseTagType::V6Tag,			LodestoneTrackerProcess } },
		{ MU8STR("minecraft:potion_contents"),			{ false,	UseTagType::V6Tag,			PotionContentsProcess } },



		{ MU8STR("minecraft:lore"),						{ false,	UseTagType::DisplayTag,		std::bind(RenameProcess,	MU8STR("Lore"),					_1, _2, _3) } },
		{ MU8STR("minecraft:map_color"),				{ false,	UseTagType::DisplayTag,		std::bind(RenameProcess,	MU8STR("MapColor"),				_1, _2, _3) } },

		{ MU8STR("minecraft:custom_name"),				{ false,	UseTagType::DisplayTag,		std::bind(DefaultProcess,	MU8STR("Name"),					ProcessCustomNameTag,			_1, _2, _3) } },
		{ MU8STR("minecraft:dyed_color"),				{ false,	UseTagType::DisplayTag,		std::bind(DefaultProcess,	MU8STR("color"),				ProcessDyedColor,				_1, _2, _3) } },
		{ MU8STR("minecraft:item_name"),				{ false,	UseTagType::DisplayTag,		std::bind(DefaultProcess,	MU8STR("Name"),					ProcessItemName,				_1, _2, _3) } },



		{ MU8STR("minecraft:block_entity_data"),		{ false,	UseTagType::BlockEntityTag,	BlockEntityDataProcess } },
		{ MU8STR("minecraft:bucket_entity_data"),		{ false,	UseTagType::BlockEntityTag,	BucketEntityDataProcess } },



		{ MU8STR("minecraft:charged_projectiles"),		{ false,	UseTagType::V6Tag,
			[](const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{



			}}
		},
		{ MU8STR("minecraft:writable_book_content"),	{ false,	UseTagType::V6Tag,
			[](const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{



			}}
		},
		{ MU8STR("minecraft:written_book_content"),		{ false,	UseTagType::V6Tag,
			[](const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{



			}}
		},



		{ MU8STR("minecraft:banner_patterns"),			{ false,	UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{
				cpdV6TileEntityData.PutString(MU8STR("id"), MU8STR("minecraft:banner"));
				
				
			}}
		},
		{ MU8STR("minecraft:bees"),						{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{
				cpdV6TileEntityData.PutString(MU8STR("id"), strItemId);


			}}
		},
		{ MU8STR("minecraft:container"),				{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{



			}}
		},
		{ MU8STR("minecraft:container_loot"),			{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{
				cpdV6TileEntityData.PutString(MU8STR("id"), strItemId);


			}}
		},
		{ MU8STR("minecraft:lock"),						{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{
				cpdV6TileEntityData.PutString(MU8STR("id"), strItemId);
				cpdV6TileEntityData.Put(MU8STR("Lock"), std::move(nodeV7TagVal));
			}}
		},
		{ MU8STR("minecraft:pot_decorations"),			{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TileEntityData) -> void
			{
				cpdV6TileEntityData.PutString(MU8STR("id"), strItemId);


			}}
		},
	};

	NBT_Type::Compound cpdBlockEntityTag;
	NBT_Type::Compound cpdDisplayTag;

	for (auto &[strV7Key, strV7Val] : cpdV7Tag)
	{
		//查找是否有匹配的处理过程
		auto itFind = mapProccess.find(strV7Key);
		if (itFind == mapProccess.end())
		{
			//不匹配直接移动处理
			cpdV6Tag.Put(strV7Key, std::move(strV7Val));
			continue;
		}

		//进行处理
		auto &mapVal = itFind->second;
		switch (mapVal.enUseTagType)
		{
		default:
		case UseTagType::V6Tag:
			mapVal.funcProcess(mapVal.bUseItemId ? strItemId : strV7Key, strV7Val, cpdV6Tag);
			break;
		case UseTagType::BlockEntityTag:
			mapVal.funcProcess(mapVal.bUseItemId ? strItemId : strV7Key, strV7Val, cpdBlockEntityTag);
			break;
		case UseTagType::DisplayTag:
			mapVal.funcProcess(mapVal.bUseItemId ? strItemId : strV7Key, strV7Val, cpdDisplayTag);
			break;
		}
	}

	if (!cpdBlockEntityTag.Empty())
	{
		cpdV6Tag.PutCompound(MU8STR("BlockEntityTag"), cpdBlockEntityTag);
	}

	if (!cpdDisplayTag.Empty())
	{
		cpdV6Tag.PutCompound(MU8STR("display"), cpdDisplayTag);
	}

	return;
}

void ProcessItems(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsList())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &listV7 = nodeV7Tag.GetList();
	auto &listV6 = nodeV6Tag.SetList();

	NBT_Type::Byte bSlot = -1;
	for (auto &itV7Entry : listV7)
	{
		++bSlot;//槽位计数，用于默认值修复

		if (!itV7Entry.IsCompound())
		{
			continue;
		}

		auto &cpdV7Entry = itV7Entry.GetCompound();
		NBT_Type::Compound cpdV6Entry;

		auto *pId = cpdV7Entry.HasString(MU8STR("id"));
		if (pId == NULL)
		{
			continue;
		}

		const auto & strId = cpdV6Entry.PutString(MU8STR("id"), std::move(*pId)).first->second.GetString();
		cpdV6Entry.PutByte(MU8STR("Count"), CopyOrElse(cpdV7Entry.HasInt(MU8STR("count")), 1));
		cpdV6Entry.PutByte(MU8STR("Slot"), CopyOrElse(cpdV7Entry.HasByte(MU8STR("Slot")), bSlot));

		auto *pV7Tag = cpdV7Entry.HasCompound(MU8STR("components"));
		if (pV7Tag != NULL)
		{
			NBT_Type::Compound cpdV6Tag;
			ProcessComponentsTag(*pV7Tag, strId, cpdV6Tag);
			cpdV6Entry.PutCompound(MU8STR("tag"), std::move(cpdV6Tag));
		}

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

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
		cpdV6Entry.PutInt(MU8STR("Color"), iColor);//插入

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
		cpdV6Entry.PutString(MU8STR("Pattern"), strPattern);//插入
	}

	return;
}

void ProcessSkullProfile(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{

	return;
}

void ProcessBlockPosDefault(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	ProcessBlockPos(nodeV7Tag, nodeV6Tag);
}

void ProcessBlockPos(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag,
	const NBT_Type::String &strX = MU8STR("X"),
	const NBT_Type::String &strY = MU8STR("Y"),
	const NBT_Type::String &strZ = MU8STR("Z"))
{
	//V7为IntArray顺序存储的xyz坐标
	//V6为Compound打包的x、y、z的Int类型成员
	if (!nodeV7Tag.IsIntArray())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//坐标只有3个
	auto &iarrBlockPos = nodeV7Tag.GetIntArray();
	if (iarrBlockPos.size() != 3)
	{
		return;
	}

	auto &cpdBlockPos = nodeV6Tag.SetCompound();
	cpdBlockPos.PutInt(strX, iarrBlockPos[0]);
	cpdBlockPos.PutInt(strY, iarrBlockPos[1]);
	cpdBlockPos.PutInt(strZ, iarrBlockPos[2]);

	return;
}

void ProcessBees(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
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

		//获取类型，并新建类型
		auto &cpdV7Entry = itV7Entry.GetCompound();
		auto &cpdV6Entry = listV6.AddBackCompound({}).first->GetCompound();

		cpdV6Entry.PutInt(MU8STR("TicksInHive"), CopyOrElse(cpdV7Entry.HasInt(MU8STR("ticks_in_hive")), 0));
		cpdV6Entry.PutInt(MU8STR("MinOccupationTicks"), CopyOrElse(cpdV7Entry.HasInt(MU8STR("min_ticks_in_hive")), 0));

		//处理实体数据转换
		auto *pFind = cpdV7Entry.HasCompound(MU8STR("entity_data"));
		if (pFind == NULL)
		{
			cpdV6Entry.PutCompound(MU8STR("EntityData"), {});//没有则插入空值返回
			continue;
		}

		//实体转换代理
		NBT_Type::Compound cpdV6EntityData;
		ProcessEntity(*pFind, cpdV6EntityData);
		cpdV6Entry.PutCompound(MU8STR("EntityData"), std::move(cpdV6EntityData));
	}

	return;
}

void ProcessSingleItem(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7Item = nodeV7Tag.GetCompound();
	auto &cpdV6Item = nodeV6Tag.SetCompound();

	auto &strId = cpdV6Item.PutString(MU8STR("id"), MoveOrElse(cpdV7Item.HasString(MU8STR("id")), MU8STR(""))).first->second.GetString();
	cpdV6Item.PutByte(MU8STR("count"), CopyOrElse(cpdV7Item.HasInt(MU8STR("Count")), 1));

	auto *pTag = cpdV7Item.HasCompound(MU8STR("components"));
	if (pTag != NULL)
	{
		NBT_Type::Compound cpdV6Tag;
		ProcessComponentsTag(*pTag, strId, cpdV6Tag);
		cpdV6Item.PutCompound(MU8STR("tag"), std::move(cpdV6Tag));
	}

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

