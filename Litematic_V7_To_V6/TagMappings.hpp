#pragma once

#include <nbt_cpp/NBT_All.hpp>
#include <unordered_map>
#include <memory>
#include "UUID_Tools.hpp"

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


struct AttributeInfo
{
public:
	const bool bIsRandomUuid;
	const union
	{
		UUID uuid;
		NBT_Type::String name;
	};
public:
	AttributeInfo(UUID _uuid)
		: bIsRandomUuid(false), uuid(std::move(_uuid))
	{}
	AttributeInfo(NBT_Type::String _name)
		: bIsRandomUuid(true), name(std::move(_name))
	{}

	AttributeInfo(const AttributeInfo &other)
		: bIsRandomUuid(other.bIsRandomUuid)
	{
		if (bIsRandomUuid)
		{
			std::construct_at(&name, other.name);
		}
		else
		{
			std::construct_at(&uuid, other.uuid);
		}
	}

	AttributeInfo(AttributeInfo &&other) noexcept
		: bIsRandomUuid(other.bIsRandomUuid)
	{
		if (bIsRandomUuid)
		{
			std::construct_at(&name, std::move(other.name));
		}
		else
		{
			std::construct_at(&uuid, std::move(other.uuid));
		}
	}

	AttributeInfo &operator=(const AttributeInfo &other) = delete;
	AttributeInfo &operator=(AttributeInfo &&other) noexcept = delete;

	~AttributeInfo(void)
	{
		if (bIsRandomUuid)
		{
			std::destroy_at(&name);
		}
		else
		{
			std::destroy_at(&uuid);
		}
	}
};

AttributeInfo AttributeInfoMap(const NBT_Type::String &strAttrId, const NBT_Type::String &strEntityId)
{
	const static std::unordered_map<NBT_Type::String, AttributeInfo> mapAttributeUUID =
	{
		//mapped uuid
		{ MU8STR("minecraft:creative_mode_block_range"),			String2Uuid<"736565D2-E1A7-403D-A3F8-1AEB3E302542">() },
		{ MU8STR("minecraft:creative_mode_entity_range"),			String2Uuid<"98491EF6-97B1-4584-AE82-71A8CC85CF73">() },
		{ MU8STR("minecraft:effect.speed"),							String2Uuid<"91AEAA56-376B-4498-935B-2F7F68070635">() },
		{ MU8STR("minecraft:effect.slowness"),						String2Uuid<"7107DE5E-7CE8-4030-940E-514C1F160890">() },
		{ MU8STR("minecraft:effect.haste"),							String2Uuid<"AF8B6E3F-3328-4C0A-AA36-5BA2BB9DBEF3">() },
		{ MU8STR("minecraft:effect.mining_fatigue"),				String2Uuid<"55FCED67-E92A-486E-9800-B47F202C4386">() },
		{ MU8STR("minecraft:effect.strength"),						String2Uuid<"648D7064-6A60-4F59-8ABE-C2C23A6DD7A9">() },
		{ MU8STR("minecraft:effect.jump_boost"),					String2Uuid<"C0105BF3-AEF8-46B0-9EBC-92943757CCBE">() },
		{ MU8STR("minecraft:effect.weakness"),						String2Uuid<"22653B89-116E-49DC-9B6B-9971489B5BE5">() },
		{ MU8STR("minecraft:effect.health_boost"),					String2Uuid<"5D6F0BA2-1186-46AC-B896-C61C5CEE99CC">() },
		{ MU8STR("minecraft:effect.absorption"),					String2Uuid<"EAE29CF0-701E-4ED6-883A-96F798F3DAB5">() },
		{ MU8STR("minecraft:effect.luck"),							String2Uuid<"03C3C89D-7037-4B42-869F-B146BCB64D2E">() },
		{ MU8STR("minecraft:effect.unluck"),						String2Uuid<"CC5AF142-2BD2-4215-B636-2605AED11727">() },
		{ MU8STR("minecraft:evil"),									String2Uuid<"6555BE74-63B3-41F1-A245-77833B3C2562">() },
		{ MU8STR("minecraft:powder_snow"),							String2Uuid<"1EAF83FF-7207-4596-B37A-D7A07B3EC4CE">() },
		{ MU8STR("minecraft:sprinting"),							String2Uuid<"662A6B8D-DA3E-4C1C-8813-96EA6097278D">() },
		{ MU8STR("minecraft:covered"),								String2Uuid<"7E0292F2-9434-48D5-A29F-9583AF7DF27F">() },
		{ MU8STR("minecraft:suffocating"),							String2Uuid<"9E362924-01DE-4DDD-A2B2-D0F7A405A174">() },
		{ MU8STR("minecraft:drinking"),								String2Uuid<"5CD17E52-A79A-43D3-A529-90FDE04B181E">() },
		{ MU8STR("minecraft:armor.boots"),							String2Uuid<"845DB27C-C624-495F-8C9F-6020A9A58B6B">() },
		{ MU8STR("minecraft:armor.leggings"),						String2Uuid<"D8499B04-0E66-4726-AB29-64469D734E0D">() },
		{ MU8STR("minecraft:armor.chestplate"),						String2Uuid<"9F3D476D-C118-4544-8365-64846904B48E">() },
		{ MU8STR("minecraft:armor.helmet"),							String2Uuid<"2AD3F246-FEE1-4E67-B886-69FD380BB150">() },
		{ MU8STR("minecraft:armor.body"),							String2Uuid<"C1C72771-8B8E-BA4A-ACE0-81A93C8928B2">() },
		{ MU8STR("minecraft:enchantment.fire_protection"),			String2Uuid<"B572ECD2-AC0C-4071-ABDE-9594AF072A37">() },
		{ MU8STR("minecraft:enchantment.blast_protection"),			String2Uuid<"40A9968F-5C66-4E2F-B7F4-2EC2F4B3E450">() },
		{ MU8STR("minecraft:enchantment.respiration"),				String2Uuid<"07A65791-F64D-4E79-86C7-F83932F007EC">() },
		{ MU8STR("minecraft:enchantment.aqua_affinity"),			String2Uuid<"60B1B7DB-FFFD-4AD0-817C-D6C6A93D8A45">() },
		{ MU8STR("minecraft:enchantment.depth_strider"),			String2Uuid<"11DC269A-4476-46C0-AFF3-9E17D7EB6801">() },
		{ MU8STR("minecraft:enchantment.soul_speed"),				String2Uuid<"87F46A96-686F-4796-B035-22E16EE9E038">() },
		{ MU8STR("minecraft:enchantment.swift_sneak"),				String2Uuid<"92437D00-C3A7-4F2E-8F6C-1F21585D5DD0">() },
		{ MU8STR("minecraft:enchantment.sweeping_edge"),			String2Uuid<"5D3D087B-DEBE-4037-B53E-D84F3FF51F17">() },
		{ MU8STR("minecraft:enchantment.efficiency"),				String2Uuid<"3CEB37C0-DB62-46B5-BD02-785457B01D96">() },
		{ MU8STR("minecraft:base_attack_damage"),					String2Uuid<"CB3F55D3-645C-4F38-A497-9C13A33DB5CF">() },
		{ MU8STR("minecraft:base_attack_speed"),					String2Uuid<"FA233E1C-4180-4865-B01B-BCCE9785ACA3">() },
		//{ MU8STR("minecraft:attacking"),							String2Uuid<"020E0DFB-87AE-4653-9556-831010E291A0">() },
		//{ MU8STR("minecraft:attacking"),							String2Uuid<"49455A49-7EC5-45BA-B886-3B90B23A1718">() },
		//{ MU8STR("minecraft:baby"),								String2Uuid<"B9766B59-9566-4402-BC1F-2EE2A276D836">() },
		//{ MU8STR("minecraft:baby"),								String2Uuid<"766BFA64-11F3-11EA-8D71-362B9E155667">() },

		//random uuid
		{ MU8STR("minecraft:random_spawn_bonus"),					MU8STR("Random spawn bonus") },
		{ MU8STR("minecraft:zombie_random_spawn_bonus"),			MU8STR("Random zombie-spawn bonus") },
		{ MU8STR("minecraft:leader_zombie_bonus"),					MU8STR("Leader zombie bonus") },
		{ MU8STR("minecraft:reinforcement_callee_charge"),			MU8STR("Zombie reinforcement callee charge") },
		{ MU8STR("minecraft:reinforcement_caller_charge"),			MU8STR("Zombie reinforcement caller charge") },
	};


	auto itFind = mapAttributeUUID.find(strAttrId);
	if (itFind != mapAttributeUUID.end())
	{
		return itFind->second;
	}
	
	//没找到，处理特殊情况
	if (strAttrId == MU8STR("minecraft:baby"))
	{
		const static std::unordered_map<NBT_Type::String, AttributeInfo> mapAttributeUUIDBaby =
		{
			{ MU8STR("minecraft:zombie"),			String2Uuid<"B9766B59-9566-4402-BC1F-2EE2A276D836">() },
			{ MU8STR("minecraft:husk"),				String2Uuid<"B9766B59-9566-4402-BC1F-2EE2A276D836">() },
			{ MU8STR("minecraft:drowned"),			String2Uuid<"B9766B59-9566-4402-BC1F-2EE2A276D836">() },
			{ MU8STR("minecraft:zombie_villager"),	String2Uuid<"B9766B59-9566-4402-BC1F-2EE2A276D836">() },
			{ MU8STR("minecraft:zombified_piglin"),	String2Uuid<"B9766B59-9566-4402-BC1F-2EE2A276D836">() },
			{ MU8STR("minecraft:piglin"),			String2Uuid<"766BFA64-11F3-11EA-8D71-362B9E155667">() },
		};

		auto itFind = mapAttributeUUIDBaby.find(strEntityId);
		return itFind != mapAttributeUUIDBaby.end() ? itFind->second : String2Uuid<"B9766B59-9566-4402-BC1F-2EE2A276D836">();//默认uuid
	}
	else if (strAttrId == MU8STR("minecraft:attacking"))
	{
		const static std::unordered_map<NBT_Type::String, AttributeInfo> mapAttributeUUIDAttacking =
		{

			{ MU8STR("minecraft:enderman"),			String2Uuid<"020E0DFB-87AE-4653-9556-831010E291A0">() },
			{ MU8STR("minecraft:zombified_piglin"),	String2Uuid<"49455A49-7EC5-45BA-B886-3B90B23A1718">() },
		};

		auto itFind = mapAttributeUUIDAttacking.find(strEntityId);
		return itFind != mapAttributeUUIDAttacking.end() ? itFind->second : String2Uuid<"020E0DFB-87AE-4653-9556-831010E291A0">();//默认uuid
	}
	else
	{
		return UUID{};//zero uuid
	}
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

NBT_Type::Int DyeColorMap(const NBT_Type::String &strColor)
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

NBT_Type::Int AxolotlVariantMap(const NBT_Type::String &strVariant)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::Int> mapAxolotlVariant =
	{
		{ MU8STR("lucy"),	0 },
		{ MU8STR("wild"),	1 },
		{ MU8STR("gold"),	2 },
		{ MU8STR("cyan"),	3 },
		{ MU8STR("blue"),	4 },
	};

	auto itFind = mapAxolotlVariant.find(strVariant);
	return itFind != mapAxolotlVariant.end() ? itFind->second : 0; // default variant
}

consteval NBT_Type::Int MakeTropicalFishPattern(NBT_Type::Int iSize, NBT_Type::Int iMarking)
{
	using UINT = std::make_unsigned_t<NBT_Type::Int>;
	using UBYTE = std::make_unsigned_t<NBT_Type::Byte>;
	return (((UINT)(UBYTE)iMarking) << 8) | (((UINT)(UBYTE)iSize) << 0);//低8bit为0或1，代表体型，高8bit为0~5(包含)，代表花纹
}

NBT_Type::Int TropicalFishPatternMap(const NBT_Type::String &strPattern)
{
	const static std::unordered_map<NBT_Type::String, NBT_Type::Int> mapTropicalFishPattern =
	{
		{ MU8STR("kob"),		MakeTropicalFishPattern(0, 0) },
		{ MU8STR("sunstreak"),	MakeTropicalFishPattern(0, 1) },
		{ MU8STR("snooper"),	MakeTropicalFishPattern(0, 2) },
		{ MU8STR("dasher"),		MakeTropicalFishPattern(0, 3) },
		{ MU8STR("brinely"),	MakeTropicalFishPattern(0, 4) },
		{ MU8STR("spotty"),		MakeTropicalFishPattern(0, 5) },

		{ MU8STR("flopper"),	MakeTropicalFishPattern(1, 0) },
		{ MU8STR("stripey"),	MakeTropicalFishPattern(1, 1) },
		{ MU8STR("glitter"),	MakeTropicalFishPattern(1, 2) },
		{ MU8STR("blockfish"),	MakeTropicalFishPattern(1, 3) },
		{ MU8STR("betty"),		MakeTropicalFishPattern(1, 4) },
		{ MU8STR("clayfish"),	MakeTropicalFishPattern(1, 5) },
	};

	auto itFind = mapTropicalFishPattern.find(strPattern);
	return itFind != mapTropicalFishPattern.end() ? itFind->second : MakeTropicalFishPattern(0, 0); // default pattern: kob
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