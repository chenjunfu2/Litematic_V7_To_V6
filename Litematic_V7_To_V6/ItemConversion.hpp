#pragma once

#include "BaseConversion.hpp"

#include "ItemMappings.hpp"

//可能递归调用，前向声明
void ProcessComponentsTag(NBT_Type::Compound &cpdV7Tag, const NBT_Type::String &strItemId, NBT_Type::Compound &cpdV6Tag, const NBT_Type::Int iV7McDataVersion);

void ProcessEnchantments(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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

void ProcessEntityTag(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	ProcessEntity(nodeV7Tag.GetCompound(), nodeV6Tag.SetCompound(), iV7McDataVersion);
	return;
}

void ProcessFireworkExplosion(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7Tag = nodeV7Tag.GetCompound();
	auto &cpdV6Tag = nodeV6Tag.SetCompound();

	auto *pShape = cpdV7Tag.HasString(MU8STR("shape"));
	if (pShape != NULL)
	{
		NBT_Type::Byte bType = FireworkShapeMap(*pShape);
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

void ProcessFireworks(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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
			ProcessFireworkExplosion(itV7Entry, nodeV6Explosion, iV7McDataVersion);
			listV6.AddBack(std::move(nodeV6Explosion));
		}

		cpdV6.PutList(MU8STR("Explosions"), std::move(listV6));
	}

	return;
}

void ProcessMapDecorations(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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

void ProcessUnbreakable(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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

void ProcessCustomData(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	//事实上，CustomData内部存储的key val在低版本是直接放在外面的，所以，解包出来的值直接合并即可
	cpdV6TagData.Merge(std::move(nodeV7TagVal.GetCompound()));
	return;
}

void ProcessLodestoneTracker(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto &cpdV7 = nodeV7TagVal.GetCompound();

	//与CustomData一致，数据存储在外面，但是名称有变化
	if (auto *pTracked = cpdV7.HasByte(MU8STR("tracked")); pTracked != NULL)
	{
		cpdV6TagData.PutByte(MU8STR("LodestoneTracked"), std::move(*pTracked));
	}

	if (auto *pTarget = cpdV7.HasCompound(MU8STR("target")); pTarget != NULL)
	{
		if (auto *pDimension = pTarget->HasString(MU8STR("dimension")); pDimension != NULL)
		{
			cpdV6TagData.PutString(MU8STR("LodestoneDimension"), std::move(*pDimension));
		}

		if (auto *pPos = pTarget->Has(MU8STR("pos")); pPos != NULL)
		{
			NBT_Node nodeV6Pos;
			ProcessBlockPos(*pPos, nodeV6Pos, iV7McDataVersion);
			cpdV6TagData.Put(MU8STR("LodestonePos"), std::move(nodeV6Pos));
		}
	}

	return;
}

void ProcessPotionContents(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto &cpdV7 = nodeV7TagVal.GetCompound();
	if (auto *pPotion = cpdV7.HasString(MU8STR("potion")); pPotion != NULL)
	{
		cpdV6TagData.PutString(MU8STR("Potion"), std::move(*pPotion));
	}
	if (auto *pCustomColor = cpdV7.HasInt(MU8STR("custom_color")); pCustomColor != NULL)
	{
		cpdV6TagData.PutInt(MU8STR("CustomPotionColor"), std::move(*pCustomColor));
	}
	if (auto *pCustomEffects = cpdV7.HasList(MU8STR("custom_effects")); pCustomEffects != NULL)
	{
		cpdV6TagData.PutList(MU8STR("custom_potion_effects"), std::move(*pCustomEffects));
	}

	return;
}

void ProcessWritableBookContent(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	auto *pPages = GetCompound(nodeV7TagVal).HasList(MU8STR("pages"));
	if (pPages == NULL)
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	NBT_Type::Compound cpdFilteredPages;
	NBT_Type::List listPages;
	NBT_Type::Int iPageNum = -1;
	for (auto &itPage : *pPages)
	{
		++iPageNum;//每次递增

		if (itPage.IsCompound())
		{
			//获取filtered与raw，filtered设置为页面号插入cpdFilteredPages，raw直接插入listPages尾部
			auto &cpdPage = itPage.GetCompound();
			auto *pRaw = cpdPage.HasString(MU8STR("raw"));
			if (pRaw == NULL)//页面不可用
			{
				--iPageNum;//恢复
				continue;//未知页面跳过
			}
			listPages.AddBackString(*pRaw);

			auto *pFiltered = cpdPage.HasString(MU8STR("filtered"));
			if (pFiltered != NULL)
			{
				auto strPageNum = MUTF8_Tool<uint8_t, char16_t, char>::U8ToMU8(std::to_string(iPageNum));
				cpdFilteredPages.PutString(strPageNum, std::move(*pFiltered));//页面号作为Key
			}
		}
		else if (itPage.IsString())
		{
			listPages.AddBack(std::move(itPage));//不解包为string然后再次封装成nbt_node，直接插入以减少开销
		}
		else
		{
			--iPageNum;//恢复
			continue;//未知页面跳过
		}
	}


	if (!cpdFilteredPages.Empty())
	{
		cpdV6TagData.PutCompound(MU8STR("filtered_pages"), std::move(cpdFilteredPages));
	}
	if (!listPages.Empty())
	{
		cpdV6TagData.PutList(MU8STR("pages"), std::move(listPages));
	}

	return;
}

void ProcessWrittenBookContent(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	//处理书的信息
	auto &cpdBook = nodeV7TagVal.GetCompound();
	if (auto *pAuthor = cpdBook.HasString(MU8STR("author")); pAuthor != NULL)
	{
		cpdV6TagData.PutString(MU8STR("author"), std::move(*pAuthor));
	}
	if (auto *pTitle = cpdBook.HasCompound(MU8STR("title")); pTitle != NULL)
	{
		cpdV6TagData.PutString(MU8STR("title"), MoveOrElse(pTitle->HasString(MU8STR("raw")), MU8STR("")));//必选段
		if (auto *pFilteredTitle = pTitle->HasString(MU8STR("filtered")); pFilteredTitle != NULL)//可选段
		{
			cpdV6TagData.PutString(MU8STR("filtered_title"), *pFilteredTitle);
		}
	}
	if (auto *pResolved = cpdBook.HasByte(MU8STR("resolved")); pResolved != NULL)
	{
		cpdV6TagData.PutByte(MU8STR("resolved"), *pResolved);
	}
	if (auto *pGeneration = cpdBook.HasInt(MU8STR("generation")); pGeneration != NULL)
	{
		cpdV6TagData.PutInt(MU8STR("generation"), *pGeneration);
	}

	//处理书的页面，使用书与笔处理过程的代理
	ProcessWritableBookContent(strV7TagKey, nodeV7TagVal, cpdV6TagData, iV7McDataVersion);

	return;
}

void ProcessDyedColor(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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
		if (listRGB.Size() != 3 ||
			listRGB.HasFloat(0) == NULL ||
			listRGB.HasFloat(1) == NULL ||
			listRGB.HasFloat(2) == NULL)
		{
			nodeV6Tag.SetInt(0xFF'FF'FF'FF);//default
			return;
		}

		auto fR = listRGB.GetFloat(0);
		auto fG = listRGB.GetFloat(1);
		auto fB = listRGB.GetFloat(2);

		auto bR = (uint8_t)(255.0 * fR);
		auto bG = (uint8_t)(255.0 * fG);
		auto bB = (uint8_t)(255.0 * fB);

		NBT_Type::Int iRGB =
			(uint32_t)255 << 24 |
			(uint32_t)bR << 16 |
			(uint32_t)bG << 8 |
			(uint32_t)bB << 0;

		nodeV6Tag.SetInt(iRGB);
	}

	return;
}

void ProcessItemName(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsString() && !nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//1.21.4及以前无需修改
	if (iV7McDataVersion <= MC_1_21_4_MINECRAFT_DATA_VERSION)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//1.21.4后修改
	NBT_Type::String strJsonTemp{};
	if (!ProcessTextComponent(nodeV7Tag, strJsonTemp))
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	nodeV6Tag.SetString(std::move(strJsonTemp));

	return;
}

void ProcessBlockEntityData(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	//直接转换到根
	ProcessTileEntity(nodeV7TagVal.GetCompound(), cpdV6TagData, iV7McDataVersion);
	return;
}

void ProcessBucketEntityData(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7TagVal.IsCompound())
	{
		cpdV6TagData.Put(strV7TagKey, std::move(nodeV7TagVal));
		return;
	}

	//直接转换到根
	ProcessEntity(nodeV7TagVal.GetCompound(), cpdV6TagData, iV7McDataVersion);
	return;
}

void ProcessChargedProjectile(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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

		auto *pId = cpdV7Entry.HasString(MU8STR("id"));
		if (pId == NULL)
		{
			continue;
		}

		const auto &strItemId = cpdV6Entry.PutString(MU8STR("id"), std::move(*pId)).first->second.GetString();
		cpdV6Entry.PutByte(MU8STR("Count"), CopyOrElse(cpdV7Entry.HasInt(MU8STR("count")), 1));

		if (auto *pV7Tag = cpdV7Entry.HasCompound(MU8STR("components")); pV7Tag != NULL)
		{
			NBT_Type::Compound cpdV6Tag;
			ProcessComponentsTag(*pV7Tag, strItemId, cpdV6Tag, iV7McDataVersion);
			cpdV6Entry.PutCompound(MU8STR("tag"), std::move(cpdV6Tag));
		}

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
}

void ProcessSingleItemNested(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7Item = nodeV7Tag.GetCompound();
	auto &cpdV6Item = nodeV6Tag.SetCompound();

	//低版本不存在，无用
	//auto iSlot = CopyOrElse(cpdV7Item.HasInt(MU8STR("slot")), 0);
	auto *pItem = cpdV7Item.HasCompound(MU8STR("item"));
	if (pItem == NULL)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//没有id，无用
	auto *pId = cpdV7Item.HasString(MU8STR("id"));
	if (pId == NULL)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

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

void ProcessItemsNested(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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

		//查找物品组件
		auto *pItem = cpdV7Entry.HasCompound(MU8STR("item"));
		if (pItem == NULL)
		{
			--bSlot;//空物品，不要递增修复计数
			continue;
		}

		//无id视为空物品
		auto *pId = pItem->HasString(MU8STR("id"));
		if (pId == NULL)
		{
			--bSlot;//空物品，不要递增修复计数
			continue;
		}

		//依次插入Id，Count和Slot
		const auto &strItemId = cpdV6Entry.PutString(MU8STR("id"), std::move(*pId)).first->second.GetString();
		cpdV6Entry.PutByte(MU8STR("Count"), CopyOrElse(pItem->HasInt(MU8STR("count")), 1));//至少要有1个
		cpdV6Entry.PutByte(MU8STR("Slot"), (NBT_Type::Byte)CopyOrElse(cpdV7Entry.HasInt(MU8STR("slot")), bSlot));//获取外层槽位，如果不存在则使用修复值

		//递归转换tag
		if (auto *pV7Tag = pItem->HasCompound(MU8STR("components")); pV7Tag != NULL)
		{
			NBT_Type::Compound cpdV6Tag;
			ProcessComponentsTag(*pV7Tag, strItemId, cpdV6Tag, iV7McDataVersion);
			cpdV6Entry.PutCompound(MU8STR("tag"), std::move(cpdV6Tag));
		}

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
}

void ProcessLootTable(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7 = nodeV7Tag.GetCompound();
	auto &cpdV6 = nodeV6Tag.SetCompound();

	if (auto *pLootTable = cpdV7.HasCompound(MU8STR("loot_table")); pLootTable != NULL)
	{
		cpdV6.Merge(std::move(*pLootTable));
	}

	if (auto *pSeed = cpdV7.HasLong(MU8STR("seed")); pSeed != NULL)
	{
		cpdV6.PutLong(MU8STR("LootTableSeed"), *pSeed);
	}

	return;
}

void ProcessItems(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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
			--bSlot;//空物品不递增计数
			continue;
		}

		const auto &strItemId = cpdV6Entry.PutString(MU8STR("id"), std::move(*pId)).first->second.GetString();
		cpdV6Entry.PutByte(MU8STR("Count"), CopyOrElse(cpdV7Entry.HasInt(MU8STR("count")), 1));
		cpdV6Entry.PutByte(MU8STR("Slot"), CopyOrElse(cpdV7Entry.HasByte(MU8STR("Slot")), bSlot));

		if (auto *pV7Tag = cpdV7Entry.HasCompound(MU8STR("components")); pV7Tag != NULL)
		{
			NBT_Type::Compound cpdV6Tag;
			ProcessComponentsTag(*pV7Tag, strItemId, cpdV6Tag, iV7McDataVersion);
			cpdV6Entry.PutCompound(MU8STR("tag"), std::move(cpdV6Tag));
		}

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
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

//物品Tag处理
void ProcessComponentsTag(NBT_Type::Compound &cpdV7Tag, const NBT_Type::String &strItemId, NBT_Type::Compound &cpdV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;
	using std::placeholders::_4;

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

	const static std::unordered_map<NBT_Type::String, MapVal_T> mapProcess =
	{
		{ MU8STR("minecraft:block_state"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("BlockStateTag"),		_1, _2, _3, _4) } },
		{ MU8STR("minecraft:instrument"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("instrument"),			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:map_id"),					{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("map"),					_1, _2, _3, _4) } },
		{ MU8STR("minecraft:recipes"),					{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("Recipes"),				_1, _2, _3, _4) } },
		{ MU8STR("minecraft:suspicious_stew_effects"),	{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("effects"),				_1, _2, _3, _4) } },
		{ MU8STR("minecraft:trim"),						{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("Trim"),					_1, _2, _3, _4) } },

		{ MU8STR("minecraft:can_break"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("CanDestroy"),			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:can_place_on"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("CanPlaceOn"),			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:custom_model_data"),		{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("CustomModelData"),		_1, _2, _3, _4) } },
		{ MU8STR("minecraft:damage"),					{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("Damage"),				_1, _2, _3, _4) } },
		{ MU8STR("minecraft:debug_stick_state"),		{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("DebugProperty"),		_1, _2, _3, _4) } },
		{ MU8STR("minecraft:note_block_sound"),			{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("note_block_sound"),		_1, _2, _3, _4) } },
		{ MU8STR("minecraft:repair_cost"),				{ false,	UseTagType::V6Tag,			std::bind(RenameProcess,	MU8STR("RepairCost"),			_1, _2, _3, _4) } },

		{ MU8STR("minecraft:attribute_modifiers"),		{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("AttributeModifiers"),	ProcessAttributes,				_1, _2, _3, _4) } },
		{ MU8STR("minecraft:bundle_contents"),			{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Items"),				ProcessItems,					_1, _2, _3, _4) } },
		{ MU8STR("minecraft:enchantments"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Enchantments"),			ProcessEnchantments,			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:stored_enchantments"),		{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("StoredEnchantments"),	ProcessEnchantments,			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:entity_data"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("EntityTag"),			ProcessEntityTag,				_1, _2, _3, _4) } },
		{ MU8STR("minecraft:firework_explosion"),		{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Explosion"),			ProcessFireworkExplosion,		_1, _2, _3, _4) } },
		{ MU8STR("minecraft:fireworks"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Fireworks"),			ProcessFireworks,				_1, _2, _3, _4) } },
		{ MU8STR("minecraft:map_decorations"),			{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Decorations"),			ProcessMapDecorations,			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:profile"),					{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("SkullOwner"),			ProcessSkullProfile,			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:unbreakable"),				{ false,	UseTagType::V6Tag,			std::bind(DefaultProcess,	MU8STR("Unbreakable"),			ProcessUnbreakable,				_1, _2, _3, _4) } },

		{ MU8STR("minecraft:custom_data"),				{ false,	UseTagType::V6Tag,			ProcessCustomData } },
		{ MU8STR("minecraft:lodestone_tracker"),		{ false,	UseTagType::V6Tag,			ProcessLodestoneTracker } },
		{ MU8STR("minecraft:potion_contents"),			{ false,	UseTagType::V6Tag,			ProcessPotionContents } },
		{ MU8STR("minecraft:writable_book_content"),	{ false,	UseTagType::V6Tag,			ProcessWritableBookContent } },
		{ MU8STR("minecraft:written_book_content"),		{ false,	UseTagType::V6Tag,			ProcessWrittenBookContent } },


		{ MU8STR("minecraft:lore"),						{ false,	UseTagType::DisplayTag,		std::bind(RenameProcess,	MU8STR("Lore"),					_1, _2, _3, _4) } },
		{ MU8STR("minecraft:map_color"),				{ false,	UseTagType::DisplayTag,		std::bind(RenameProcess,	MU8STR("MapColor"),				_1, _2, _3, _4) } },

		{ MU8STR("minecraft:custom_name"),				{ false,	UseTagType::DisplayTag,		std::bind(DefaultProcess,	MU8STR("Name"),					ProcessCustomNameTag,			_1, _2, _3, _4) } },
		{ MU8STR("minecraft:dyed_color"),				{ false,	UseTagType::DisplayTag,		std::bind(DefaultProcess,	MU8STR("color"),				ProcessDyedColor,				_1, _2, _3, _4) } },
		{ MU8STR("minecraft:item_name"),				{ false,	UseTagType::DisplayTag,		std::bind(DefaultProcess,	MU8STR("Name"),					ProcessItemName,				_1, _2, _3, _4) } },



		{ MU8STR("minecraft:block_entity_data"),		{ false,	UseTagType::BlockEntityTag,	ProcessBlockEntityData } },
		{ MU8STR("minecraft:bucket_entity_data"),		{ false,	UseTagType::BlockEntityTag,	ProcessBucketEntityData } },



		{ MU8STR("minecraft:charged_projectiles"),		{ false,	UseTagType::V6Tag,
			[](const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion) -> void
			{
				cpdV6TagData.PutByte(MU8STR("Charged"), 1);//boolean=true
				DefaultProcess(MU8STR("ChargedProjectiles"), ProcessChargedProjectile, strV7TagKey, nodeV7TagVal, cpdV6TagData, iV7McDataVersion);
			}}
		},



		{ MU8STR("minecraft:banner_patterns"),			{ false,	UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion) -> void
			{
				cpdV6TagData.PutString(MU8STR("id"), MU8STR("minecraft:banner"));
				DefaultProcess(MU8STR("Patterns"), ProcessPatterns, strV7TagKey, nodeV7TagVal, cpdV6TagData, iV7McDataVersion);
			}}
		},
		{ MU8STR("minecraft:bees"),						{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion) -> void
			{
				cpdV6TagData.PutString(MU8STR("id"), strItemId);
				DefaultProcess(MU8STR("Bees"), ProcessBees, MU8STR(""), nodeV7TagVal, cpdV6TagData, iV7McDataVersion);
			}}
		},
		{ MU8STR("minecraft:container"),				{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion) -> void
			{
				if (strItemId.find(MU8STR("decorated_pot")) != strItemId.npos)
				{
					DefaultProcess(MU8STR("item"), ProcessSingleItemNested, MU8STR(""), nodeV7TagVal, cpdV6TagData, iV7McDataVersion);
				}
				else
				{
					DefaultProcess(MU8STR("Items"), ProcessItemsNested, MU8STR(""), nodeV7TagVal, cpdV6TagData, iV7McDataVersion);
				}

				bool bShulker = strItemId.find(MU8STR("shulker")) != strItemId.npos;
				cpdV6TagData.PutString(MU8STR("id"), bShulker ? MU8STR("minecraft:shulker_box") : strItemId);
			}}
		},
		{ MU8STR("minecraft:container_loot"),			{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion) -> void
			{
				cpdV6TagData.PutString(MU8STR("id"), strItemId);
				DefaultProcess(MU8STR("LootTable"), ProcessLootTable, MU8STR(""), nodeV7TagVal, cpdV6TagData, iV7McDataVersion);
			}}
		},
		{ MU8STR("minecraft:lock"),						{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion) -> void
			{
				cpdV6TagData.PutString(MU8STR("id"), strItemId);
				cpdV6TagData.Put(MU8STR("Lock"), std::move(nodeV7TagVal));
			}}
		},
		{ MU8STR("minecraft:pot_decorations"),			{ true,		UseTagType::BlockEntityTag,
			[](const NBT_Type::String &strItemId, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion) -> void
			{
				cpdV6TagData.PutString(MU8STR("id"), strItemId);
				RenameProcess(MU8STR("sherds"), MU8STR(""), nodeV7TagVal, cpdV6TagData, iV7McDataVersion);
			}}
		},
	};

	NBT_Type::Compound cpdBlockEntityTag;
	NBT_Type::Compound cpdDisplayTag;

	for (auto &[strV7Key, strV7Val] : cpdV7Tag)
	{
		//查找是否有匹配的处理过程
		auto itFind = mapProcess.find(strV7Key);
		if (itFind == mapProcess.end())
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
			mapVal.funcProcess(mapVal.bUseItemId ? strItemId : strV7Key, strV7Val, cpdV6Tag, iV7McDataVersion);
			break;
		case UseTagType::BlockEntityTag:
			mapVal.funcProcess(mapVal.bUseItemId ? strItemId : strV7Key, strV7Val, cpdBlockEntityTag, iV7McDataVersion);
			break;
		case UseTagType::DisplayTag:
			mapVal.funcProcess(mapVal.bUseItemId ? strItemId : strV7Key, strV7Val, cpdDisplayTag, iV7McDataVersion);
			break;
		}
	}

	if (!cpdBlockEntityTag.Empty())
	{
		cpdV6Tag.PutCompound(MU8STR("BlockEntityTag"), std::move(cpdBlockEntityTag));
	}

	if (!cpdDisplayTag.Empty())
	{
		cpdV6Tag.PutCompound(MU8STR("display"), std::move(cpdDisplayTag));
	}

	//现在物品id已经在V6中
	auto *pItemId = cpdV6Tag.HasString(MU8STR("id"));
	if (pItemId != NULL)
	{
		(void)ItemIdMap(*pItemId, *pItemId, iV7McDataVersion);//如果映射成功，那么会自动替换，否则不变
	}

	return;
}
