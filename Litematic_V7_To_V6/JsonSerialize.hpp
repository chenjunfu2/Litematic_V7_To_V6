#pragma once

#include <nbt_cpp/NBT_Node.hpp>
#include <nbt_cpp/NBT_Node_View.hpp>
#include <format>
#include <limits>
#include <cmath>
#include <string>
#include <vector>
#include <type_traits>
#include <unordered_set>

class NBTJsonHelper
{
	NBTJsonHelper(void) = delete;
	~NBTJsonHelper(void) = delete;
public:
	struct NoSortCompound
	{};

	template<bool bAscending = true>
	struct DefaultCompoundSort
	{
		std::vector<NBT_Type::Compound::Const_Iterator> operator()(const NBT_Type::Compound &cpdSort)
		{
			return cpdSort.KeySortIt<bAscending>();
		}
	};


	template<auto IsBoolKey, typename SortPolicy = DefaultCompoundSort<true>>
	requires(std::is_function_v<std::remove_cvref_t<decltype(IsBoolKey)>>)
	static NBT_Type::String JsonSerialize(const NBT_Node_View<true> nRoot)
	{
		NBT_Type::String sRet{};
		JsonSerializeSwitch<IsBoolKey, true, SortPolicy>(nRoot, sRet);
		return sRet;
	}

	template<typename T, typename STR_T>
	requires(NBT_Type::IsNumericType_V<T> && (std::is_same_v<STR_T, NBT_Type::String> || std::is_same_v<STR_T, std::string>))
	static void NumericToDecString(const T &value, STR_T &result)
	{
		std::string tmp{};
		if constexpr (NBT_Type::IsFloatingType_V<T>)
		{
			if (std::isnan(value))
			{
				tmp = "0";//NaN -> 0
			}
			else if (std::isinf(value))
			{
				bool bSign = std::signbit(value);//是否为负
				tmp = std::format("{:.{}g}", bSign ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max(), std::numeric_limits<T>::max_digits10);//不能用min()，那是最小正正规数
			}
			else
			{
				tmp = std::format("{:.{}g}", value, std::numeric_limits<T>::max_digits10);
			}
		}
		else if constexpr (NBT_Type::IsIntegerType_V<T>)
		{
			tmp = std::format("{:d}", value);
		}
		else
		{
			static_assert(false, "T type unknown");
		}

		if constexpr (std::is_same_v<STR_T, NBT_Type::String>)
		{
			result += (NBT_Type::String)tmp;//转换为NBT字符串
		}
		else if constexpr (std::is_same_v<STR_T, std::string>)
		{
			result += tmp;
		}
		else
		{
			static_assert(false, "STR_T type unknown");
		}
	}

	static NBT_Type::String EscapeJsonString(const NBT_Type::String &strRaw)
	{
		NBT_Type::String strEscape{};
		strEscape.reserve(strRaw.size());//预分配

		using UC = std::make_unsigned_t<std::remove_cvref_t<decltype(strRaw[0])>>;
		bool bIsC0 = false;
		for (const auto &it : strRaw)
		{
			if (bIsC0)
			{
				if ((UC)it == (UC)0x80)
				{
					strEscape.append(MU8STRV("\\u0000"));
					bIsC0 = false;
					continue;
				}
				else
				{
					strEscape.push_back((UC)0xC0);
					bIsC0 = false;
				}
			}

			switch ((UC)it)
			{
			case (UC)'\"':
			case (UC)'\\':
				strEscape.push_back('\\');
				strEscape.push_back(it);
				break;
				/*case '/': //此项无必要*/
			case (UC)'\b':
				strEscape.append(MU8STRV("\\b"));
				break;
			case (UC)'\f':
				strEscape.append(MU8STRV("\\f"));
				break;
			case (UC)'\n':
				strEscape.append(MU8STRV("\\n"));
				break;
			case (UC)'\r':
				strEscape.append(MU8STRV("\\r"));
				break;
			case (UC)'\t':
				strEscape.append(MU8STRV("\\t"));
				break;
			case (UC)0xC0:
				bIsC0 = true;
				continue;
				break;
			default:
				if ((UC)it > (UC)0x1F)
				{
					strEscape.push_back(it);
				}
				else//0x00~0x1F
				{
					strEscape.append(MU8STRV("\\u00"));
					strEscape.push_back(MU8STRV("0123456789ABCDEF")[((UC)it >> 4) & 0x0F]);
					strEscape.push_back(MU8STRV("0123456789ABCDEF")[((UC)it >> 0) & 0x0F]);
				}
				break;
			}
		}

		if (bIsC0)
		{
			strEscape.push_back((UC)0xC0);
			bIsC0 = false;
		}

		return strEscape;
	}

	static NBT_Type::String RawStringToJsonString(const NBT_Type::String &strRaw)
	{
		//为空改为引号包围的空字符串
		if (strRaw.empty())
		{
			return MU8STR("{\"text\":\"\"}");
		}

		//不为空则添加双引号并转义内容
		auto strTemp = EscapeJsonString(strRaw);

		NBT_Type::String strJson{};
		//预分配
		constexpr size_t szBaseTextSize = MU8STRV("{\"text\":\"\"}").size();//MSVC在连续临时量求值发生崩溃，此处必须首先保存为constexpr
		strJson.reserve(szBaseTextSize + strTemp.size() * sizeof(*strTemp.data()));//然后在下面进行求值

		//拼接
		strJson.append(MU8STRV("{\"text\":\""));
		strJson.append(std::move(strTemp));
		strJson.append(MU8STRV("\"}"));

		//返回转义完成的字符串
		return strJson;
	}



	//首次调用默认为true，二次调用开始内部主动变为false
	template<auto IsBoolKey, bool bRoot, typename SortPolicy>//首次使用NBT_Node_View解包，后续直接使用NBT_Node引用免除额外初始化开销
	static void JsonSerializeSwitch(std::conditional_t<bRoot, const NBT_Node_View<true> &, const NBT_Node &>nRoot, NBT_Type::String &sRet)
	{
		auto tag = nRoot.GetTag();
		switch (tag)
		{
		case NBT_TAG::End:
			{
				sRet += MU8STR("null");
			}
			break;
		case NBT_TAG::Byte:
			{
				NumericToDecString(nRoot.template Get<NBT_Type::Byte>(), sRet);
			}
			break;
		case NBT_TAG::Short:
			{
				NumericToDecString(nRoot.template Get<NBT_Type::Short>(), sRet);
			}
			break;
		case NBT_TAG::Int:
			{
				NumericToDecString(nRoot.template Get<NBT_Type::Int>(), sRet);
			}
			break;
		case NBT_TAG::Long:
			{
				NumericToDecString(nRoot.template Get<NBT_Type::Long>(), sRet);
			}
			break;
		case NBT_TAG::Float:
			{
				NumericToDecString(nRoot.template Get<NBT_Type::Float>(), sRet);
			}
			break;
		case NBT_TAG::Double:
			{
				NumericToDecString(nRoot.template Get<NBT_Type::Double>(), sRet);
			}
			break;
		case NBT_TAG::ByteArray:
			{
				const auto &arr = nRoot.template Get<NBT_Type::ByteArray>();

				sRet += MU8STR("[");

				for (const auto &it : arr)
				{
					NumericToDecString(it, sRet);
					sRet += ',';
				}
				if (arr.size() != 0)
				{
					sRet.pop_back();//删掉最后一个逗号
				}

				sRet += ']';
			}
			break;
		case NBT_TAG::IntArray:
			{
				const auto &arr = nRoot.template Get<NBT_Type::IntArray>();

				sRet += MU8STR("[");

				for (const auto &it : arr)
				{
					NumericToDecString(it, sRet);
					sRet += ',';
				}
				if (arr.size() != 0)
				{
					sRet.pop_back();//删掉最后一个逗号
				}

				sRet += ']';
			}
			break;
		case NBT_TAG::LongArray:
			{
				const auto &arr = nRoot.template Get<NBT_Type::LongArray>();

				sRet += MU8STR("[");

				for (const auto &it : arr)
				{
					NumericToDecString(it, sRet);
					sRet += ',';
				}
				if (arr.size() != 0)
				{
					sRet.pop_back();//删掉最后一个逗号
				}

				sRet += ']';
			}
			break;
		case NBT_TAG::String:
			{
				sRet += '\"';
				sRet += EscapeJsonString(nRoot.template Get<NBT_Type::String>());
				sRet += '\"';
			}
			break;
		case NBT_TAG::List:
			{
				const auto &list = nRoot.template Get<NBT_Type::List>();
				sRet += '[';
				for (const auto &it : list)
				{
					JsonSerializeSwitch<IsBoolKey, false, SortPolicy>(it, sRet);
					sRet += ',';
				}

				if (list.Size() != 0)
				{
					sRet.pop_back();//删掉最后一个逗号
				}
				sRet += ']';
			}
			break;
		case NBT_TAG::Compound://需要打印缩进的地方
			{
				const auto &cpd = nRoot.template Get<NBT_Type::Compound>();
				sRet += '{';

				if constexpr (std::is_same_v<SortPolicy, NoSortCompound>)
				{
					for (const auto &it : cpd)
					{
						sRet += '\"';
						sRet += EscapeJsonString(it.first);
						sRet += MU8STR("\":");
						if (it.second.GetTag() == NBT_TAG::Byte && IsBoolKey(it.first))
						{
							sRet += it.second.Get<NBT_Type::Byte>() ? MU8STR("true") : MU8STR("false");
						}
						else
						{
							JsonSerializeSwitch<IsBoolKey, false, SortPolicy>(it.second, sRet);
						}
						sRet += ',';
					}
				}
				else
				{
					std::vector<NBT_Type::Compound::Const_Iterator> vSort = SortPolicy{}(cpd);

					for (const auto &it : vSort)
					{
						sRet += '\"';
						sRet += EscapeJsonString(it->first);
						sRet += MU8STR("\":");
						if (it->second.GetTag() == NBT_TAG::Byte && IsBoolKey(it->first))
						{
							sRet += it->second.Get<NBT_Type::Byte>() ? MU8STR("true") : MU8STR("false");
						}
						else
						{
							JsonSerializeSwitch<IsBoolKey, false, SortPolicy>(it->second, sRet);
						}
						sRet += ',';
					}
				}

				if (cpd.Size() != 0)
				{
					sRet.pop_back();//删掉最后一个逗号
				}
				sRet += '}';
			}
			break;
		default:
			{
				//Ignore Error
			}
			break;
		}
	}
};

