#pragma once

#include <nbt_cpp/NBT_All.hpp>
#include <util/MD5.hpp>
#include "TagMappings.hpp"
#include "ItemMappings.hpp"
#include "JsonSerialize.hpp"

#include <unordered_map>
#include <functional>
#include <utility>
#include <charconv>
#include <type_traits>

//定义
#define MC_1_21_4_MINECRAFT_DATA_VERSION 4189

template<typename T, typename V>
requires(std::is_same_v<std::decay_t<T>, std::decay_t<V>> || std::is_constructible_v<T, V>)
T CopyOrElse(const T *p, V &&d)
{
	return p != NULL ? *p : std::forward<V>(d);
}

template<typename T, typename V>
requires(std::is_same_v<std::decay_t<T>, std::decay_t<V>> || std::is_constructible_v<T, V>)
T MoveOrElse(T *p, V &&d)
{
	return p != NULL ? std::move(*p) : std::forward<V>(d);
}

template<typename SS, typename ST>
requires
(
	NBT_String_Helper::Is_NBT_String_Or_View<std::remove_cvref_t<SS>> &&
	NBT_String_Helper::Is_NBT_String_Or_View<std::remove_cvref_t<ST>>
)
bool TryRenameMove(NBT_Type::Compound &cpdSource, SS &&strSource, NBT_Type::Compound &cpdTarget, ST &&strTarget, NBT_TAG tagType = NBT_TAG::ENUM_END)
{
	if (NBT_Node *pSources = cpdSource.Has(std::forward<SS>(strSource)); pSources != NULL)
	{
		if (tagType != NBT_TAG::ENUM_END && pSources->GetTag() != tagType)
		{
			return false;
		}

		cpdTarget.Put(std::forward<ST>(strTarget), std::move(*pSources));
		return true;
	}
	else
	{
		return false;
	}
}

template<typename SS, typename ST, typename F>
requires
(
	NBT_String_Helper::Is_NBT_String_Or_View<std::remove_cvref_t<SS>> &&
	NBT_String_Helper::Is_NBT_String_Or_View<std::remove_cvref_t<ST>> &&
	std::is_invocable_v<F, NBT_Node &, NBT_Node &> &&
	std::same_as<std::invoke_result_t<F, NBT_Node &, NBT_Node &>, bool>
)// use: [](NBT_Node &v7Val, NBT_Node &v6Val) -> bool {};
bool TryRenameTransfer(NBT_Type::Compound &cpdSource, SS &&strSource, NBT_Type::Compound &cpdTarget, ST &&strTarget, F &&funcTransfer)
{
	if (NBT_Node *pSources = cpdSource.Has(std::forward<SS>(strSource)); pSources != NULL)
	{
		NBT_Node tmpTarget{};
		if (!std::invoke(funcTransfer, *pSources, tmpTarget))
		{
			return false;
		}

		cpdTarget.Put(std::forward<ST>(strTarget), std::move(tmpTarget));
		return true;
	}
	else
	{
		return false;
	}
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
void ProcessItems(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion);
void ProcessSingleItem(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion);
void ProcessComponentsTag(NBT_Type::Compound &cpdV7Tag, const NBT_Type::String &strItemId, NBT_Type::Compound &cpdV6Tag, const NBT_Type::Int iV7McDataVersion);
void ProcessTextComponentConversion(NBT_Node &nodeTextComponent, NBT_Node &nodeTextConversion, const NBT_Type::Int iV7McDataVersion);
NBT_Type::String MapEntityTypeIdForDisplay(NBT_Type::String &strId, const NBT_Type::Int iV7McDataVersion);


//等价java.util.UUID.nameUUIDFromBytes
void NameUuidFromBytes(const void *data, size_t len, NBT_Type::IntArray &iaUuidList)
{
	uint8_t digest[16];
	MD5::FastMd5((const uint8_t *)data, len, digest);//这东西实现搞死我了
	
	digest[6] = (uint8_t)((digest[6] & 0x0F) | 0x30);//version = 3
	digest[8] = (uint8_t)((digest[8] & 0x3F) | 0x80);//variant = IETF

	//直接按大端把 16 字节拆成 4 个 int32（等价于原 msb/lsb 再拆分）
	auto rd32 = [&](int i) -> int32_t
	{
		return 
			((uint32_t)(digest[i + 0]) << 24) |
			((uint32_t)(digest[i + 1]) << 16) |
			((uint32_t)(digest[i + 2]) <<  8) |
			((uint32_t)(digest[i + 3]) <<  0);
	};

	//清理预分配
	iaUuidList.clear();
	iaUuidList.reserve(4);
	//插入
	iaUuidList.push_back(rd32(0));	//msb >> 32
	iaUuidList.push_back(rd32(4));	//msb
	iaUuidList.push_back(rd32(8));	//lsb >> 32
	iaUuidList.push_back(rd32(12));	//lsb

}

bool Uuid2String(NBT_Type::IntArray &iaUuidList, NBT_Type::String &strUuid)
{
	if (iaUuidList.size() != 4)
	{
		return false;
	}

	uint32_t time_low = iaUuidList[0];
	uint16_t time_mid = ((uint32_t)iaUuidList[1] >> 16) & 0xFFFFu;
	uint16_t time_hi_and_version = (uint32_t)iaUuidList[1] & 0xFFFFu;
	uint16_t clock_seq = ((uint32_t)iaUuidList[2] >> 16) & 0xFFFFu;
	uint64_t node = (((uint64_t)((uint32_t)iaUuidList[2] & 0xFFFFu) << 32) | (uint64_t)(uint32_t)iaUuidList[3]) & 0xFFFFFFFFFFFFull;

	strUuid = (NBT_Type::String)std::format(
		"{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
		time_low, time_mid, time_hi_and_version, clock_seq, node
	);

	return true;
}

bool String2Uuid(NBT_Type::String &strUuid, NBT_Type::IntArray &iaUuidList)
{
	if (strUuid.size() != (8 + 1 + (4 + 1) * 3 + 12))
	{
		return false;
	}

	//首先验证连字符
	//8-4-4-4-12
	//8 13 18 23
	auto *pStrUuid = strUuid.data();//只取一次，减少string重复判断缓存模式开销
	if (pStrUuid[ 8] != '-' ||
		pStrUuid[13] != '-' ||
		pStrUuid[18] != '-' ||
		pStrUuid[23] != '-')
	{
		return false;
	}

	auto GetCharByte =
	[](uint32_t &data, MUTF8_Char_Type ch, MUTF8_Char_Type cl) -> bool
	{
		static constexpr auto charMap = //字符映射值
		[]() -> auto
		{
			std::array<uint8_t, std::numeric_limits<MUTF8_Char_Type>::max() + 1> arr{};
			std::ranges::fill(arr, (uint8_t)-1);

			uint8_t u8Val = 0;
			for (const auto &it : MU8STRV("0123456789ABCDEFabcdef"))
			{
				arr[it] = u8Val++;
				if (it == 'F')
				{
					u8Val = 0x0A;
				}
			}

			return arr;
		}();

		//校验，必须存在映射
		if (charMap[ch] == (uint8_t)-1 || charMap[cl] == (uint8_t)-1)
		{
			return false;
		}

		//组合字节并放入低位
		uint8_t byte =
			charMap[ch] << 4 |//4h
			charMap[cl] << 0;//4l

		data <<= 8;
		data |= (uint32_t)byte;

		return true;
	};

	uint32_t u32Uuid[4]{};
	static constexpr auto indexMap = //索引映射
	[]() -> auto
	{
		std::array<size_t, 32> arr{};

		size_t idx = 0;
		for (auto &it : arr)
		{
			switch (idx)
			{
			case 8: case 13:
			case 18: case 23:
				++idx;
			}

			it = idx++;
		}

		return arr;
	}();

	for (size_t i = 0; i < 4 * 4; ++i)//16->u32Uuid 通过/4赋值0~3，indexMap通过i*2+0与i*2+1映射索引跑全32字节（跳过'-'）
	{
		if (!GetCharByte(u32Uuid[i / 4], pStrUuid[indexMap[i * 2 + 0]], pStrUuid[indexMap[i * 2 + 1]]))
		{
			return false;
		}
	}
	
	//转换到nbt数组
	iaUuidList.clear();
	iaUuidList.reserve(4);
	std::ranges::transform(u32Uuid, std::back_inserter(iaUuidList),
		[](const uint32_t &u) -> auto { return std::bit_cast<NBT_Type::Int>(u); }
	);

	return true;
}


/*
关于文本组件：
游戏不会使用列表形式进行序列化，仅使用字符串形式和复合标签形式存储与传输。
字符串格式是纯文本组件的简写形式。在实际使用中，字符串格式等价于复合标签格式的{text: <字符串>}。
如果一个文本组件所有样式均为默认值、不包含子组件且为纯文本组件，游戏在序列化或持久化此组件时会直接使用字符串格式而非复合标签格式。
*/

void ProcessClickEvent(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	static const std::unordered_map<NBT_Type::String, NBT_Type::String> mapActionType =
	{
		{ MU8STR("open_url"),			MU8STR("url") },
		{ MU8STR("open_file"),			MU8STR("path") },
		{ MU8STR("run_command"),		MU8STR("command") },
		{ MU8STR("suggest_command"),	MU8STR("command") },
		{ MU8STR("copy_to_clipboard"),	MU8STR("value") },
		{ MU8STR("change_page"),		MU8STR("page") },
	};

	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7ClickEvent = nodeV7Tag.GetCompound();

	//从action匹配
	auto *pAction = cpdV7ClickEvent.HasString(MU8STRV("action"));
	if (pAction == NULL)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//找不到：未知字段、新字段等情况，不处理，原始保留
	auto itMappedActionTypeKey = mapActionType.find(*pAction);
	if (itMappedActionTypeKey == mapActionType.end())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//找到，不存在，原始保留，不处理
	auto pActionValue = cpdV7ClickEvent.Has(itMappedActionTypeKey->second);
	if (pActionValue == NULL)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	NBT_Type::String strValue{};

	//目前只可能是这两种类型，否则失败
	if (pActionValue->IsString())
	{
		strValue = std::move(pActionValue->GetString());
	}
	else if (pActionValue->IsInt())
	{
		strValue = (NBT_Type::String)std::format("{:d}", pActionValue->GetInt());
	}
	else
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//全部验证通过，转换
	auto &cpdV6ClickEvent = nodeV6Tag.SetCompound();
	cpdV6ClickEvent.PutString(MU8STRV("action"), std::move(*pAction));
	cpdV6ClickEvent.PutString(MU8STRV("value"), std::move(strValue));

	return;
}

void ProcessHoverEvent(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	enum class ActionType : uint8_t
	{
		Text,
		Item,
		Entity,
	};

	static const std::unordered_map<NBT_Type::String, ActionType> mapActionType =
	{
		{ MU8STR("show_text"),			ActionType::Text },
		{ MU8STR("show_item"),			ActionType::Item },
		{ MU8STR("show_entity"),		ActionType::Entity },
	};


	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7ClickEvent = nodeV7Tag.GetCompound();

	//从action匹配
	auto *pAction = cpdV7ClickEvent.HasString(MU8STRV("action"));
	if (pAction == NULL)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//找不到：未知字段、新字段等情况，不处理，原始保留
	auto itMappedActionType = mapActionType.find(*pAction);
	if (itMappedActionType == mapActionType.end())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	switch (itMappedActionType->second)
	{
	case ActionType::Text:
		{
			auto *pValue = cpdV7ClickEvent.Has(MU8STRV("value"));
			if (pValue == NULL)
			{
				nodeV6Tag = std::move(nodeV7Tag);
				return;
			}

			NBT_Node nodeV6Contents{};
			switch (pValue->GetTag())
			{
			case NBT_TAG::String:
				{
					nodeV6Contents.SetString(std::move(pValue->GetString()));
				}
				break;
			case NBT_TAG::Compound://递归到文本组件主根（嵌套）
			case NBT_TAG::List://列表
				{
					ProcessTextComponentConversion(*pValue, nodeV6Contents, iV7McDataVersion);
				}
				break;
			default://必须有，否则失败
				{
					nodeV6Tag = std::move(nodeV7Tag);
					return;
				}
				break;
			}

			//准备就绪，一次性插入
			auto &cpdV6HoverEvent = nodeV6Tag.SetCompound();
			cpdV6HoverEvent.PutString(MU8STRV("action"), std::move(*pAction));
			cpdV6HoverEvent.Put(MU8STRV("contents"), std::move(nodeV6Contents));
		}
		break;
	case ActionType::Item:
		{
			auto *pId = cpdV7ClickEvent.HasString(MU8STRV("id"));
			if (pId == NULL)
			{
				nodeV6Tag = std::move(nodeV7Tag);
				return;
			}

			auto *pCount = cpdV7ClickEvent.HasInt(MU8STRV("count"));//可选，1.20.1也可选
			auto *pComponents = cpdV7ClickEvent.HasCompound(MU8STRV("components"));//可选1，1.20.1也可选
			
			//处理contents
			NBT_Node nodeV6Contents{};
			auto &cpdV6Contents = nodeV6Contents.SetCompound();

			//可选转换
			if (pCount != NULL)
			{
				cpdV6Contents.PutInt(MU8STR("count"), *pCount);
			}
			if (pComponents != NULL)
			{
				NBT_Type::Compound cpdV6Tag;
				ProcessComponentsTag(*pComponents, *pId, cpdV6Tag, iV7McDataVersion);
				cpdV6Contents.PutCompound(MU8STR("tag"), std::move(cpdV6Tag));
			}

			//最后映射物品ID并插入，因为前面ProcessComponentsTag用到id，不能提前移动，同时id映射必须在处理之后，又因为要消拷贝，所以放最后
			(void)ItemIdMap(*pId, *pId, iV7McDataVersion);
			cpdV6Contents.PutString(MU8STR("id"), std::move(*pId));

			//准备就绪，一次性插入
			auto &cpdV6HoverEvent = nodeV6Tag.SetCompound();
			cpdV6HoverEvent.PutString(MU8STRV("action"), std::move(*pAction));
			cpdV6HoverEvent.Put(MU8STRV("contents"), std::move(nodeV6Contents));
		}
		break;
	case ActionType::Entity:
		{
			auto *pId = cpdV7ClickEvent.HasString(MU8STRV("id"));
			if (pId == NULL)
			{
				nodeV6Tag = std::move(nodeV7Tag);
				return;
			}

			auto *pUuid = cpdV7ClickEvent.Has(MU8STRV("uuid"));
			if (pUuid == NULL)
			{
				nodeV6Tag = std::move(nodeV7Tag);
				return;
			}

			//处理contents
			NBT_Node nodeV6Contents{};
			auto &cpdV6Contents = nodeV6Contents.SetCompound();

			//别问为什么映射这么奇怪，id是uuid，type才是id，name是组件，问mojang去，无敌了
			if (pUuid->IsString())
			{
				cpdV6Contents.PutString(MU8STR("id"), std::move(pUuid->GetString()));
			}
			else if (pUuid->IsIntArray())
			{
				NBT_Type::String strUuid{};
				if (!Uuid2String(pUuid->GetIntArray(), strUuid))
				{
					nodeV6Tag = std::move(nodeV7Tag);
					return;
				}
				cpdV6Contents.PutString(MU8STR("id"), std::move(strUuid));
			}
			else//必须要是至少一个正确类型，否则失败
			{
				nodeV6Tag = std::move(nodeV7Tag);
				return;
			}

			//前面失败提前返回，此插入必须在之后
			cpdV6Contents.PutString(MU8STR("type"), MapEntityTypeIdForDisplay(*pId, iV7McDataVersion));
			
			auto *pName = cpdV7ClickEvent.Has(MU8STRV("name"));//可选
			if (pName != NULL)
			{
				switch (pName->GetTag())
				{
				case NBT_TAG::String:
					{
						cpdV6Contents.PutString(MU8STR("name"), std::move(pName->GetString()));
					}
					break;
				case NBT_TAG::Compound:
				case NBT_TAG::List:
					{
						NBT_Node nodeV6TextComponent{};
						ProcessTextComponentConversion(*pName, nodeV6TextComponent, iV7McDataVersion);
						cpdV6Contents.Put(MU8STR("name"), std::move(nodeV6TextComponent));
					}
					break;
				default:
					{
						//ignored
					}
					break;
				}
			}

			//准备就绪，一次性插入
			auto &cpdV6HoverEvent = nodeV6Tag.SetCompound();
			cpdV6HoverEvent.PutString(MU8STRV("action"), std::move(*pAction));
			cpdV6HoverEvent.Put(MU8STRV("contents"), std::move(nodeV6Contents));
		}
		break;
	default:
		{
			nodeV6Tag = std::move(nodeV7Tag);
			return;
		}
		break;
	}

	return;
}


//进行映射，因为所有文本注解都从这里转换，MC WIKI已经无法提供更多信息，从Java多版本源代码分析格式变动
void ProcessTextComponentConversion(NBT_Node &nodeTextComponent, NBT_Node &nodeTextConversion, const NBT_Type::Int iV7McDataVersion)
{
	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;
	using std::placeholders::_4;

	static const std::unordered_map<NBT_Type::String, MapValFunc_T> mapProcess =
	{
		{ MU8STR("click_event"),	std::bind(DefaultProcess,	MU8STR("clickEvent"),		ProcessClickEvent,					_1, _2, _3, _4)	},
		{ MU8STR("hover_event"),	std::bind(DefaultProcess,	MU8STR("hoverEvent"),		ProcessHoverEvent,					_1, _2, _3, _4)	},
		{ MU8STR("extra"),			std::bind(DefaultProcess,	MU8STR("extra"),			ProcessTextComponentConversion,		_1, _2, _3, _4)	},//此处递归，因为extra是递归结构，内部可以继续存储TextComponentTag
		{ MU8STR("with"),			std::bind(DefaultProcess,	MU8STR("with"),				ProcessTextComponentConversion,		_1, _2, _3, _4)	},//此处递归，因为with是递归结构，内部可以继续存储TextComponentTag
		{ MU8STR("separator"),		std::bind(DefaultProcess,	MU8STR("separator"),		ProcessTextComponentConversion,		_1, _2, _3, _4)	},//此处递归，因为separator是递归结构，内部可以继续存储TextComponentTag
	};

	auto *ProcessAll =//使用+退化为函数指针
	+[](NBT_Type::Compound &cpdV7TextData, NBT_Type::Compound &cpdV6TextData, const NBT_Type::Int iV7McDataVersion) -> void
	{
		for (auto &[itV7TagKey, itV7TagVal] : cpdV7TextData)
		{
			//查找是否有匹配的处理过程
			auto itFind = mapProcess.find(itV7TagKey);
			if (itFind == mapProcess.end())
			{
				//不匹配直接移动处理
				cpdV6TextData.Put(itV7TagKey, std::move(itV7TagVal));
				continue;
			}

			//进行处理
			auto &funcProcess = itFind->second;
			funcProcess(itV7TagKey, itV7TagVal, cpdV6TextData, iV7McDataVersion);
		}
	};

	switch (nodeTextComponent.GetTag())
	{
	case NBT_TAG::Compound:
		{
			ProcessAll(GetCompound(nodeTextComponent), nodeTextConversion.SetCompound(), iV7McDataVersion);
		}
		break;
	case NBT_TAG::List://对每个list项递归处理
		{
			auto &listTextComponent = GetList(nodeTextComponent);
			auto &listTextConversion = nodeTextConversion.SetList();

			for (auto &itText : listTextComponent)
			{
				ProcessTextComponentConversion(itText, listTextConversion.AddBack(NBT_Node{}), iV7McDataVersion);//插入并转换
			}
		}
		break;
	case NBT_TAG::String:
	default://unknown type
		{
			nodeTextConversion = std::move(nodeTextComponent);
			return;
		}
		break;
	}
}


bool IsBoolKey(const NBT_Type::String &key)
{
	static const std::unordered_set<NBT_Type::String> setBoolKeys =
	{
		MU8STR("bold"),
		MU8STR("italic"),
		MU8STR("underlined"),
		MU8STR("strikethrough"),
		MU8STR("obfuscated"),
		MU8STR("interpret"),
	};

	return setBoolKeys.contains(key);
}

//调用语义，如果目标只接受字符串，那么直接完整调用，否则如果目标本身是字符串列表，就应该外部解包后挨个调用
bool ProcessTextComponent(NBT_Node &nodeTextComponent, NBT_Type::String &strRawJsonText, const NBT_Type::Int iV7McDataVersion)
{
	NBT_Node nodeTextConversion{};
	ProcessTextComponentConversion(nodeTextComponent, nodeTextConversion, iV7McDataVersion);//进行全部的映射转换

	switch (nodeTextConversion.GetTag())
	{
	case NBT_TAG::String:
		{
			strRawJsonText = NBTJsonHelper::RawStringToJsonString(GetString(nodeTextConversion));
		}
		break;
	case NBT_TAG::Compound:
		{
			auto &cpdText = GetCompound(nodeTextConversion);
			strRawJsonText = NBTJsonHelper::JsonSerialize<IsBoolKey>(cpdText);//转化为json字符串（自动转义特殊字符）
		}
		break;
	case NBT_TAG::List:
		{
			auto &listText = GetList(nodeTextConversion);
			strRawJsonText = NBTJsonHelper::JsonSerialize<IsBoolKey>(listText);//转化为json字符串（自动转义特殊字符）
		}
		break;
	default:
		return false;
		break;
	}

	return true;
}

void ProcessAttributeModifiers1(NBT_Type::List &listV7Tag, NBT_Type::List &listV6Tag, const NBT_Type::Int iV7McDataVersion)
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

enum class HideFlag : NBT_Type::Int
{
	ENCHANTMENTS	= 1 << 0,	//附魔名
	MODIFIERS		= 1 << 1,	//属性修饰符
	UNBREAKABLE		= 1 << 2,	//无法破坏
	CAN_DESTROY		= 1 << 3,	//可破坏
	CAN_PLACE		= 1 << 4,	//可放置
	ADDITIONAL		= 1 << 5,	//物品自身附加提示
	DYE				= 1 << 6,	//染色
	UPGRADES		= 1 << 7,	//盔甲纹饰
};


void TrySetHideFlag(NBT_Type::Compound &cpdV7, NBT_Type::Compound &cpdV6, HideFlag tag)
{
	if (auto *pShow = cpdV7.HasByte(MU8STRV("show_in_tooltip")); pShow != NULL && *pShow == 0)
	{
		auto &iHideFlags = cpdV6.TryPutInt(MU8STRV("HideFlags"), 0).first->second.GetInt();
		iHideFlags |= std::underlying_type_t<HideFlag>(tag);
	}
}

void ProcessModifiers(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	if (!nodeV7Tag.IsCompound())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &cpdV7 = nodeV7Tag.GetCompound();
	auto &cpdV6 = nodeV6Tag.SetCompound();

	
	TryRenameMove(cpdV7, MU8STRV("name"), cpdV6, MU8STRV("Name"), NBT_TAG::String);
	TryRenameMove(cpdV7, MU8STRV("amount"), cpdV6, MU8STRV("Amount"), NBT_TAG::Double);

	TryRenameTransfer(cpdV7, MU8STRV("type"), cpdV6, MU8STRV("AttributeName"), 
		[](NBT_Node &v7Val, NBT_Node &v6Val) -> bool
		{
			if (!v7Val.IsString())
			{
				return false;
			}

			v6Val.SetString(AttributeNameMap(v7Val.GetString()));
			return true;
		}
	);
	
	TryRenameTransfer(cpdV7, MU8STRV("uuid"), cpdV6, MU8STRV("UUID"),
		[](NBT_Node &v7Val, NBT_Node &v6Val) -> bool
		{
			if (v7Val.IsString())
			{
				NBT_Type::IntArray uuid{};
				if (!String2Uuid(v7Val.GetString(), uuid))
				{
					return false;
				}
				v6Val.SetIntArray(std::move(uuid));
			}
			else if (v7Val.IsIntArray())
			{
				v6Val = std::move(v7Val);
			}
			else
			{
				return false;
			}

			return true;
		}
	);

	TryRenameTransfer(cpdV7, MU8STRV("operation"), cpdV6, MU8STRV("Operation"),
		[](NBT_Node &v7Val, NBT_Node &v6Val) -> bool
		{
			if (!v7Val.IsString())
			{
				return false;
			}

			v6Val.SetInt(AttributeOperationMap(v7Val.GetString()));
			return true;
		}
	);

	TryRenameTransfer(cpdV7, MU8STRV("slot"), cpdV6, MU8STRV("Slot"),
		[](NBT_Node &v7Val, NBT_Node &v6Val) -> bool
		{
			if (!v7Val.IsString())
			{
				return false;
			}

			//1.20.4-中省略等同于any
			if (v7Val.GetString() == MU8STRV("any"))
			{
				return false;//故意返回false让它不要插入，并非失败
			}

			v6Val = std::move(v7Val);
			return true;
		}
	);

}

void ProcessAttributeModifiers(const NBT_Type::String &strV7TagKey, NBT_Node &nodeV7TagVal, NBT_Type::Compound &cpdV6TagData, const NBT_Type::Int iV7McDataVersion)
{
	//V6 Key = MU8STR("AttributeModifiers")
	//TODO:TryRenameMove等辅助函数，减少代码
	//TODO:std bind全改模板编译期绑定
	//TODO:MUSTR尽量全改MU8STRV，NBT库全线支持透明查找
	NBT_Type::List *pModifiers = NULL;

	if (nodeV7TagVal.IsCompound())
	{
		auto &cpdV7 = nodeV7TagVal.GetCompound();
		pModifiers = cpdV7.HasList(MU8STRV("modifiers"));
		TrySetHideFlag(cpdV7, cpdV6TagData, HideFlag::MODIFIERS);//可能隐藏，尝试设置flag
	}
	else if (nodeV7TagVal.IsList())//列表show默认true，无需设置hide flag
	{
		pModifiers = &nodeV7TagVal.GetList();
	}

	if (pModifiers == NULL)
	{
		cpdV6TagData.Put(MU8STR("AttributeModifiers"), std::move(nodeV7TagVal));
		return;
	}

	NBT_Type::List listV6Modifiers{};
	for (auto &it : *pModifiers)
	{
		NBT_Node tmpV6Node{};
		ProcessModifiers(it, tmpV6Node, iV7McDataVersion);
		listV6Modifiers.AddBack(std::move(tmpV6Node));
	}

	if (!listV6Modifiers.Empty())
	{
		cpdV6TagData.PutList(MU8STR("AttributeModifiers"), std::move(listV6Modifiers));
	}

	return;
}

//TODO:改实体专用
//TODO:nameUUIDFromBytes实现（需要MD5）
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
				ProcessAttributeModifiers1(nodeV7Val.GetList(), listV6, iV7McDataVersion);
				return;
			}
		}

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
			ProcessAttributeModifiers1(*pModifiers, listV6Modifiers, iV7McDataVersion);
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
			iColor = DyeColorMap(*pstrColor);
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

void ProcessTextComponentTag(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	//1.21.4及以前无需修改（具体问题参考ProcessTextComponentListTag中的说明）
	if (iV7McDataVersion <= MC_1_21_4_MINECRAFT_DATA_VERSION)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//1.21.4后修改
	NBT_Type::String strJsonTemp{};
	if (!ProcessTextComponent(nodeV7Tag, strJsonTemp, iV7McDataVersion))//目标只能是字符串，不论是str、cpd、list都直接序列化为字符串
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	nodeV6Tag.SetString(std::move(strJsonTemp));

	return;
}

//lore专用
void ProcessTextComponentListTag(NBT_Node &nodeV7Tag, NBT_Node &nodeV6Tag, const NBT_Type::Int iV7McDataVersion)
{
	//1.21.4及以前无需修改（实际上1.21.4之前，虽然还是json字符串，但是内部部分key已经部分根据1.20.5+的nbt规则修改了，如果需要
	//严格映射，必须写json反解析器到nbt，然后重新递归传入ProcessTextComponent之类的例程，我*你*的mojang，劳资不搞了，此处直接忽略！
	if (iV7McDataVersion <= MC_1_21_4_MINECRAFT_DATA_VERSION)
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	//1.21.4后修改
	if (!nodeV7Tag.IsList())
	{
		nodeV6Tag = std::move(nodeV7Tag);
		return;
	}

	auto &listV7 = nodeV7Tag.GetList();
	auto &listV6 = nodeV6Tag.SetList();

	for (auto &it : listV7)
	{
		NBT_Type::String strJsonTemp{};
		if (!ProcessTextComponent(it, strJsonTemp, iV7McDataVersion))//目标只能是字符串，不论是str、cpd、list都直接序列化为字符串
		{
			listV6.AddBack(std::move(it));
			continue;
		}

		listV6.AddBackString(std::move(strJsonTemp));
	}

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
			if (!itTexture.IsString())
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
