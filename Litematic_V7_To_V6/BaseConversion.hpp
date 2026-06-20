#pragma once

#include "nbt_cpp/NBT_All.hpp"
#include "Mappings.hpp"

#include <unordered_map>
#include <functional>
#include <utility>

//定义
#define MC_1_21_4_MINECRAFT_DATA_VERSION 4189

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

//实际转换
using TagProcessFunc_T = std::function<void(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)>;
//用于Map值
using MapValFunc_T = std::function<void(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)>;

//通用处理
void DefaultProcess(const NBT_Type::String &strNewKey, TagProcessFunc_T funcTagProcess, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	NBT_Node nodeV6TagVal;
	funcTagProcess(nodeV7TagVal, nodeV6TagVal, iV7McDataVersion);
	cpdV6TagData.Put(strNewKey, std::move(nodeV6TagVal));
};

//重命名处理
void RenameProcess(const NBT_Type::String &strNewKey, const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	cpdV6TagData.Put(strNewKey, std::move(nodeV7TagVal));
}

//前向声明
void ProcessEntity(NBT_Type::Compound &cpdV7EntityData, NBT_Type::Compound &cpdV6EntityData, const NBT_Type::Int iV7McDataVersion);
void ProcessTileEntity(NBT_Type::Compound &cpdV7TileEntityData, NBT_Type::Compound &cpdV6TileEntityData, const NBT_Type::Int iV7McDataVersion);
void ProcessComponentsTag(NBT_Type::Compound &cpdV7Tag, const NBT_Type::String &strItemId, NBT_Type::Compound &cpdV6Tag, const NBT_Type::Int iV7McDataVersion);

/*
关于文本组件：
游戏不会使用列表形式进行序列化，仅使用字符串形式和复合标签形式存储与传输。
字符串格式是纯文本组件的简写形式。在实际使用中，字符串格式等价于复合标签格式的{text: <字符串>}。
如果一个文本组件所有样式均为默认值、不包含子组件且为纯文本组件，游戏在序列化或持久化此组件时会直接使用字符串格式而非复合标签格式。
*/
NBT_Type::String EscapeString(const NBT_Type::String &strRaw)
{
	NBT_Type::String strEscape{};
	strEscape.reserve(strRaw.size());//预分配
	for (const auto &it : strRaw)
	{
		switch (it)
		{
		case '\"':
		case '\\':
			strEscape.push_back('\\');
			break;
			//因为mc会直接处理这些空白字符的原始形式，仅需要手动处理可显示字符的转义
			/*case '/':
			case '\b':
			case '\f':
			case '\n':
			case '\r':
			case '\t':*/
		default:
			//不添加转义
			break;
		}

		strEscape.push_back(it);
	}

	return strEscape;
}

/*
//根据告示牌NBT最终序列化结果，事实上仅存储原始Json
//并不需要任何二次转义，游戏内看到的多层转义只是NBT显示导致的转义

NBT_Type::String DoubleEscapeString(const NBT_Type::String &strRaw)
{
	//进行两次转义，因为json解析需要消费一次转义，snbt解析也要消费一次转义

	//因为只要发生转义，必然插入额外字符，所以判断转义前后的大小，如果不变，那么就说明没有进行过转义，跳过第二次无效转义
	auto strFirst = EscapeString(strRaw);
	if (strFirst.size() == strRaw.size())
	{
		return strFirst;//返回转义串（这里可以享受NRVO，假设这里返回原始串——虽然与转义串等价，则会导致二次拷贝，并且浪费已经进行过拷贝的转义串。)
	}

	auto strSecond = EscapeString(strFirst);//二次转义
	return strSecond;
}
*/
NBT_Type::String ToJsonString(const NBT_Type::String &strRaw)
{
	//为空改为引号包围的空字符串
	if (strRaw.empty())
	{
		return MU8STR("{\"text\":\"\"}");
	}

	//不为空则添加双引号并转义内容
	auto strTemp = EscapeString(strRaw);

	NBT_Type::String strJson{};
	//预分配
	strJson.reserve(sizeof(MU8STR("{\"text\":\"\"}")) + strTemp.size() * sizeof(*strTemp.data()));

	//拼接
	strJson.append(MU8STR("{\"text\":\""));
	strJson.append(std::move(strTemp));
	strJson.append(MU8STR("\"}"));

	//返回转义完成的字符串
	return strJson;
}

bool ProcessTextComponent(NBT_Node &nodeTextComponent, NBT_Type::String &strRawJsonText)
{
	switch (nodeTextComponent.GetTag())
	{
	case NBT_TAG::String:
		{
			strRawJsonText = ToJsonString(GetString(nodeTextComponent));
		}
		break;
	case NBT_TAG::Compound:
		{
			//因为NBT库序列化不会添加任何转义，转义仅由用户侧完成，所以需要转义后再进行SNBT序列化
			auto &cpdText = GetCompound(nodeTextComponent);

			//如果存在，那么转义
			auto pstrText = cpdText.HasString(MU8STR("text"));
			if (pstrText != NULL)
			{
				*pstrText = EscapeString(*pstrText);
			}

			//最后把转义完成的进行序列化
			strRawJsonText = NBT_Helper::Serialize<NBT_Helper::DefaultCompoundSort<true>, false, true>(cpdText);
		}
		break;
	default:
		return false;
		break;
	}

	return true;
}

void ProcessAttributeModifiers(NBT_Type::List &listV7Tag, NBT_Type::List &listV6Tag, const NBT_Type::Int iV7McDataVersion)
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

		NBT_Type::Int iOperation = 0;//default
		if (auto *pOperation = cpdV7Entry.HasString(MU8STR("operation")); pOperation != NULL)
		{
			iOperation = AttributeOperationMap(*pOperation);
		}
		cpdV6Entry.PutInt(MU8STR("Operation"), iOperation);

		if (auto *pUUID = cpdV7Entry.HasIntArray(MU8STR("UUID")); pUUID != NULL)
		{
			cpdV6Entry.PutIntArray(MU8STR("UUID"), std::move(*pUUID));
		}

		listV6Tag.AddBack(std::move(cpdV6Entry));
	}
}

void ProcessAttributes(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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
				ProcessAttributeModifiers(nodeV7Val.GetList(), listV6, iV7McDataVersion);
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
			ProcessAttributeModifiers(*pModifiers, listV6Modifiers, iV7McDataVersion);
			if (!listV6Modifiers.Empty())
			{
				cpdV6Entry.PutList(MU8STR("Modifiers"), std::move(listV6Modifiers));
			}
		}

		listV6.AddBackCompound(std::move(cpdV6Entry));
	}

	return;
}

void ProcessBlockPos(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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
	cpdBlockPos.PutInt(MU8STR("X"), iarrBlockPos[0]);
	cpdBlockPos.PutInt(MU8STR("Y"), iarrBlockPos[1]);
	cpdBlockPos.PutInt(MU8STR("Z"), iarrBlockPos[2]);

	return;
}

void ProcessBees(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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
		auto &cpdV6Entry = listV6.AddBackCompound({}).GetCompound();

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
		ProcessEntity(*pFind, cpdV6EntityData, iV7McDataVersion);
		cpdV6Entry.PutCompound(MU8STR("EntityData"), std::move(cpdV6EntityData));
	}

	return;
}

void ProcessPatterns(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsList())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	NBT_Type::List &listV7 = nodeV7Tag.GetList();
	NBT_Type::List &listV6 = nodeV6Tag.SetList();

	for (auto &itEntry : listV7)
	{
		if (!itEntry.IsCompound())
		{
			continue;
		}

		auto &cpdV7Entry = itEntry.GetCompound();
		auto &cpdV6Entry = listV6.AddBackCompound({}).GetCompound();

		//查找并映射颜色
		NBT_Type::Int iColor = 0;
		auto *pstrColor = cpdV7Entry.HasString(MU8STR("color"));
		if (pstrColor != NULL)
		{
			iColor = BannerColorMap(*pstrColor);
		}
		cpdV6Entry.PutInt(MU8STR("Color"), iColor);//插入

		//查找并映射图样
		NBT_Type::String strPattern = MU8STR("b");
		auto *pstrPattern = cpdV7Entry.HasString(MU8STR("pattern"));
		if (pstrPattern != NULL)
		{
			strPattern = BannerPatternMap(*pstrPattern);
		}
		cpdV6Entry.PutString(MU8STR("Pattern"), strPattern);//插入
	}

	return;
}

void ProcessCustomNameTag(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
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

void ProcessSkullProfile(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7 = nodeV7Tag.GetCompound();
	auto &cpdV6 = nodeV6Tag.SetCompound();

	//UUID
	if (auto *pId = cpdV7.HasIntArray(MU8STR("id")); pId != NULL)
	{
		cpdV6.PutIntArray(MU8STR("Id"), std::move(*pId));
	}
	//Name
	if (auto *pName = cpdV7.HasString(MU8STR("name")); pName != NULL)
	{
		cpdV6.PutString(MU8STR("Name"), std::move(*pName));
	}

	auto *pProperties = cpdV7.Has(MU8STR("properties"));
	if (pProperties == NULL)
	{
		return;
	}

	NBT_Type::List listTextures;
	if (pProperties->IsList())
	{
		for (auto &itEntry : pProperties->GetList())
		{
			if (!itEntry.IsCompound())
			{
				continue;
			}

			auto &cpdV7Entry = itEntry.GetCompound();
			NBT_Type::Compound cpdV6Entry;

			//必须有名称且为纹理
			auto *pName = cpdV7Entry.HasString(MU8STR("name"));
			if (pName == NULL || *pName != MU8STRV("textures"))
			{
				continue;
			}

			//必须有纹理数据
			auto *pValue = cpdV7Entry.HasString(MU8STR("value"));
			if (pValue == NULL)
			{
				continue;
			}
			cpdV6Entry.PutString(MU8STR("Value"), *pValue);

			//可选签名
			if (auto *pSignature = cpdV7Entry.HasString(MU8STR("signature")); pSignature != NULL)
			{
				cpdV6Entry.PutString(MU8STR("Signature"), *pSignature);
			}

			listTextures.AddBackCompound(std::move(cpdV6Entry));
		}
	}
	else if (pProperties->IsCompound())
	{
		auto *pTextures = GetCompound(*pProperties).HasList(MU8STR("textures"));
		if (pTextures == NULL)
		{
			return;//不存在样式
		}

		for (auto &itTexture : *pTextures)
		{
			if (itTexture.IsString())
			{
				continue;
			}

			auto &strTexture = itTexture.GetString();
			NBT_Type::Compound cpdV6Entry;

			cpdV6Entry.PutString(MU8STR("Value"), std::move(strTexture));
			listTextures.AddBackCompound(std::move(cpdV6Entry));
		}
	}
	else//无法解析的类型
	{
		return;
	}

	if (!listTextures.Empty())
	{
		cpdV6.PutCompound(MU8STR("Properties"), NBT_Type::Compound{ {MU8STR("textures"), NBT_Node{ std::move(listTextures) }} });
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
