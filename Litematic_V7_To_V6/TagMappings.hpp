#pragma once

#include <nbt_cpp/NBT_All.hpp>
#include <unordered_map>

NBT_Type::String AttributeNameMap(const NBT_Type::String &strAttrName)
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

NBT_Type::Int AttributeOperationMap(const NBT_Type::String &strOperation)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::Int> mapOperation =
	{
		{ MU8STR("add_value"),				0 },
		{ MU8STR("add_multiplied_base"),	1 },
		{ MU8STR("add_multiplied_total"),	2 },
	};

	auto itFind = mapOperation.find(strOperation);
	return itFind != mapOperation.end() ? itFind->second : 0; // default
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
	return itFind != mapDecorationType.end() ? itFind->second : 0; // default
}

NBT_Type::Byte FireworkShapeMap(const NBT_Type::String &strShape)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::Byte> mapShape =
	{
		{ MU8STR("small_ball"),	0 },
		{ MU8STR("large_ball"),	1 },
		{ MU8STR("star"),		2 },
		{ MU8STR("creeper"),	3 },
		{ MU8STR("burst"),		4 },
	};

	auto itFind = mapShape.find(strShape);
	return itFind != mapShape.end() ? itFind->second : 0; // default
}

NBT_Type::Int BannerColorMap(const NBT_Type::String &strColor)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::Int> mapColor =
	{
		{MU8STR("white"),		0},		{MU8STR("orange"),	1},		{MU8STR("magenta"),	2},		{MU8STR("light_blue"),	3},
		{MU8STR("yellow"),		4},		{MU8STR("lime"),	5},		{MU8STR("pink"),	6},		{MU8STR("gray"),		7},
		{MU8STR("light_gray"),	8},		{MU8STR("cyan"),	9},		{MU8STR("purple"),	10},	{MU8STR("blue"),		11},
		{MU8STR("brown"),		12},	{MU8STR("green"),	13},	{MU8STR("red"),		14},	{MU8STR("black"),		15}
	};

	auto itFind = mapColor.find(strColor);
	return itFind != mapColor.end() ? itFind->second : 0; // default
}

NBT_Type::String BannerPatternMap(const NBT_Type::String &strPattern)
{
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

	auto itFind = mapPattern.find(strPattern);
	return itFind != mapPattern.end() ? itFind->second : MU8STR("b"); // default
}

NBT_Type::Int BeaconEffectMap(const NBT_Type::String &strEffect)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::Int> mapBeaconEffect =
	{
		{ MU8STR("minecraft:speed"),				1 },
		{ MU8STR("minecraft:haste"),				3 },
		{ MU8STR("minecraft:strength"),				5 },
		{ MU8STR("minecraft:jump_boost"),			8 },
		{ MU8STR("minecraft:regeneration"),			10 },
		{ MU8STR("minecraft:resistance"),			11 },
	};

	auto itFind = mapBeaconEffect.find(strEffect);
	return itFind != mapBeaconEffect.end() ? itFind->second : 1; // default speed
}

enum class HandArmorSlot
{
	mainhand,
	offhand,
	feet,
	legs,
	chest,
	head,
	body,
	saddle,
	unknown,
};

HandArmorSlot GetHandArmorSlot(const NBT_Type::String &strSlot)
{
	const static std::unordered_map<NBT_Type::String, HandArmorSlot> mapSlot =
	{
		{ MU8STR("mainhand"),	HandArmorSlot::mainhand },
		{ MU8STR("offhand"),	HandArmorSlot::offhand },
		{ MU8STR("feet"),		HandArmorSlot::feet },
		{ MU8STR("legs"),		HandArmorSlot::legs },
		{ MU8STR("chest"),		HandArmorSlot::chest },
		{ MU8STR("head"),		HandArmorSlot::head },
		{ MU8STR("body"),		HandArmorSlot::body },
		{ MU8STR("saddle"),		HandArmorSlot::saddle },
	};

	auto it = mapSlot.find(strSlot);
	return it != mapSlot.end() ? it->second : HandArmorSlot::unknown;
}