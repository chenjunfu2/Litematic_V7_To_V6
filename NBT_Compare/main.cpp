#include <nbt_cpp/NBT_All.hpp>
#include <util/CodeTimer.hpp>
#include <util/CPP_Helper.h>
#include <util/MyAssert.hpp>

#include <unordered_set>
#include <ranges>

bool EnableVirtualTerminalProcessing(void) noexcept;

// 颜色宏定义
#define COLOR_RED		"\033[91m"
#define COLOR_GREEN		"\033[92m"
#define COLOR_YELLOW	"\033[93m"
#define COLOR_BLUE		"\033[94m"
#define COLOR_CYAN		"\033[96m"
#define COLOR_RESET		"\033[0m"
#define COLOR_BOLD		"\033[1m"

//找到一个唯一文件名
std::string GenerateUniqueFilename(const std::string &sBeg, const std::string &sEnd, uint32_t u32TryCount = 10)//默认最多重试10次
{
	while (u32TryCount != 0)
	{
		//时间用[]包围
		auto tmpPath = std::format("{}[{}]{}", sBeg, CodeTimer::GetSystemTime(), sEnd);//获取当前系统时间戳作为中间的部分
		if (!NBT_IO::IsFileExist(tmpPath))
		{
			return tmpPath;
		}

		//等几ms在继续
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		--u32TryCount;
	}

	//次数到上限直接返回空
	return std::string{};
}

class NBT_Compare
{
	DELETE_CSTC(NBT_Compare);
	DELETE_DSTC(NBT_Compare);

public:
	enum class DiffInfo : uint8_t
	{
		NoDiff = 0,
		DiffTag,
		DiffVal,
		DiffLen,
		DiffKey,
		Enum_End,
	};

	struct Report
	{
		std::string strPath;
		DiffInfo enDiffInfo;
		std::string strDiffInfo;
	};

private:
	const static inline char *const pDiffInfoStr[] =
	{
		"No Different",
		"Different Tag",
		"Different Value",
		"Different Length",
		"Different CompoundKey",
	};

public:
	static const char *GetDiffTypeInfo(DiffInfo di)
	{
		auto index = std::underlying_type_t<DiffInfo>(di);
		if (index >= std::underlying_type_t<DiffInfo>(DiffInfo::Enum_End))
		{
			return "Unknown";
		}

		return pDiffInfoStr[index];
	}


private:
	template<typename T>
	requires(std::is_same_v<T, size_t> || std::is_same_v<T, std::string>)
	static void PushPath(std::vector<std::string> &listNbtPath, T tNewSeg)
	{
		if constexpr (std::is_same_v<T, size_t>)
		{
			std::string strNew;
			strNew += '[';
			strNew += std::to_string(tNewSeg);
			strNew += ']';

			listNbtPath.emplace_back(std::move(strNew));
		}
		else if constexpr(std::is_same_v<T, std::string>)
		{
			listNbtPath.emplace_back('.' + tNewSeg);
		}
		else
		{
			static_assert(false, "?");
		}
	}

	static void PopPath(std::vector<std::string> &listNbtPath)
	{
		listNbtPath.pop_back();
	}

	static std::string ConnectionPath(const std::vector<std::string> &listNbtPath)
	{
		std::string tmp;
		for (auto &it : listNbtPath)
		{
			tmp.append(it);
		}

		return tmp;
	}

	template<typename T>
	static std::string GenInfo(const T &tL, const T &tR)
	{
		if constexpr (std::is_same_v<T, NBT_Type::String>)
		{
			return std::format(COLOR_GREEN "L: \"{}\"\n" COLOR_YELLOW "R: \"{}\"" COLOR_RESET, tL.ToCharTypeUTF8(), tR.ToCharTypeUTF8());
		}
		else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<std::decay_t<T>, const char *>)
		{
			return std::format(COLOR_GREEN "L: {}\n" COLOR_YELLOW "R: {}" COLOR_RESET, tL, tR);
		}
		else
		{
			return std::format(COLOR_GREEN "L: [{}]\n" COLOR_YELLOW "R: [{}]" COLOR_RESET, tL, tR);
		}
	}

private:

	static bool CompareCompoundType(const NBT_Type::Compound &cpdLeft, const NBT_Type::Compound &cpdRight, std::vector<Report> &listReports, std::vector<std::string> &listNbtPath)
	{
		bool bRet = true;

		std::unordered_set<NBT_Type::String> strSet;
		for (auto &[key, _] : cpdLeft)
		{
			strSet.insert(key);
		}
		for (auto &[key, _] : cpdRight)
		{
			strSet.insert(key);
		}

		std::vector<NBT_Type::String> vSort;
		vSort.reserve(strSet.size());
		for (auto &it : strSet)
		{
			vSort.push_back(it);
		}

		std::sort(vSort.begin(), vSort.end(),
			[](const auto &l, const auto &r) -> bool
			{
				return l < r;
			}
		);

		for (auto &it : vSort)
		{
			auto lHas = cpdLeft.Has(it);
			auto rHas = cpdRight.Has(it);
			if (lHas && !rHas)
			{
				std::string strKey = it.ToCharTypeUTF8();

				listReports.emplace_back(
					ConnectionPath(listNbtPath),
					DiffInfo::DiffKey,
					GenInfo(std::format("Has Key \"{}\"", strKey), std::format("Mis Key \"{}\"", strKey)));
				bRet = false;
			}
			else if (!lHas && rHas)
			{
				std::string strKey = it.ToCharTypeUTF8();

				listReports.emplace_back(
					ConnectionPath(listNbtPath),
					DiffInfo::DiffKey,
					GenInfo(std::format("Mis Key \"{}\"", strKey), std::format("Has Key \"{}\"", strKey)));
				bRet = false;
			}
			else
			{
				MyAssert(lHas && rHas, "What the fu*k?");

				PushPath(listNbtPath, it.ToCharTypeUTF8());
				bRet = (uint8_t)bRet & (uint8_t)CompareDetailsImpl(*lHas, *rHas, listReports, listNbtPath);
				PopPath(listNbtPath);
			}
		}

		return bRet;
	}

	static bool CompareListType(const NBT_Type::List &listLeft, const NBT_Type::List &listRight, std::vector<Report> &listReports, std::vector<std::string> &listNbtPath)
	{
		bool bRet = true;
		if (listLeft.Size() != listRight.Size())//大小不同，先继续尝试
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffLen,
				GenInfo(listLeft.Size(), listRight.Size()));
			bRet = false;
		}

		if (listLeft.GetTag() != listRight.GetTag())
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffTag,
				GenInfo(std::format("[{}]", NBT_Type::GetTypeName(listLeft.GetTag())), std::format("[{}]", NBT_Type::GetTypeName(listRight.GetTag()))));
			return false;//成员类型不同直接不用比较
		}

		//比较最少的部分
		size_t szMin = std::min(listLeft.Size(), listRight.Size());
		for (auto i : std::views::iota((size_t)0, szMin))
		{
			PushPath(listNbtPath, i);
			bRet = (uint8_t)bRet & (uint8_t)CompareDetailsImpl(listLeft[i], listRight[i], listReports, listNbtPath);
			PopPath(listNbtPath);
		}

		return bRet;
	}

	static bool CompareStringType(const NBT_Type::String &strLeft, const NBT_Type::String &strRight, std::vector<Report> &listReports, std::vector<std::string> &listNbtPath)
	{
		if (strLeft != strRight)
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffVal,
				GenInfo(strLeft, strRight));
			return false;
		}

		return true;
	}

	template<typename T>
	static bool CompareArrayType(const T &arrayLeft, const T &arrayRight, std::vector<Report> &listReports, std::vector<std::string> &listNbtPath)
	{
		bool bRet = true;
		if (arrayLeft.size() != arrayRight.size())
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffLen,
				GenInfo(arrayLeft.size(), arrayRight.size()));
			bRet = false;
		}

		if (arrayLeft != arrayRight)
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffVal,
				GenInfo(NBT_Helper::Serialize(arrayLeft), NBT_Helper::Serialize(arrayRight)));
			return false;
		}

		return bRet;
	}

	template<typename T>
	static bool CompareBuiltinType(const T &builtinLeft, const T &builtinRight, std::vector<Report> &listReports, std::vector<std::string> &listNbtPath)
	{
		using Raw_T = NBT_Type::BuiltinRawType_T<T>;

		if (std::bit_cast<Raw_T>(builtinLeft) != std::bit_cast<Raw_T>(builtinRight))
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffVal,
				GenInfo(std::bit_cast<Raw_T>(builtinLeft), std::bit_cast<Raw_T>(builtinRight)));
			return false;
		}

		return true;
	}

	template<bool bRoot = false>
	static bool CompareDetailsImpl(std::conditional_t<bRoot, const NBT_Node_View<true> &, const NBT_Node &> nodeLeft, std::conditional_t<bRoot, const NBT_Node_View<true> &, const NBT_Node &> nodeRight, std::vector<Report> &listReports, std::vector<std::string> &listNbtPath)
	{
		NBT_TAG tagType = nodeLeft.GetTag();
		if (tagType != nodeRight.GetTag())
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffTag,
				GenInfo(std::format("[{}]", NBT_Type::GetTypeName(nodeLeft.GetTag())), std::format("[{}]", NBT_Type::GetTypeName(nodeRight.GetTag()))));
			return false;
		}

		switch (tagType)
		{
		case NBT_TAG::End:
			return true;
			break;
		case NBT_TAG::Byte:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::Byte>;
				return CompareBuiltinType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::Short:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::Short>;
				return CompareBuiltinType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::Int:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::Int>;
				return CompareBuiltinType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::Long:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::Long>;
				return CompareBuiltinType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::Float:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::Float>;
				return CompareBuiltinType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::Double:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::Double>;
				return CompareBuiltinType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::ByteArray:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::ByteArray>;
				return CompareArrayType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::String:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::String>;
				return CompareStringType(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::List:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::List>;
				return CompareListType(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::Compound:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::Compound>;
				return CompareCompoundType(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::IntArray:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::IntArray>;
				return CompareArrayType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::LongArray:
			{
				using Type = NBT_Type::TagToType_T<NBT_TAG::LongArray>;
				return CompareArrayType<Type>(nodeLeft.Get<Type>(), nodeRight.Get<Type>(), listReports, listNbtPath);
			}
			break;
		case NBT_TAG::ENUM_END:
		default:
			return false;
			break;
		}

		return false;
	}

public:
	static bool CompareDetails(const NBT_Node_View<true> nodeLeft, const NBT_Node_View<true> nodeRight, std::vector<Report> &listReports)
	{
		std::vector<std::string> listNbtPath{ {"[Root]"}};
		return CompareDetailsImpl<true>(nodeLeft, nodeRight, listReports, listNbtPath);
	}

};



int main(int argc, char *argv[])
{
	(void)EnableVirtualTerminalProcessing();

	if (argc != 3)
	{
		printf(COLOR_BLUE "Use:\n>[%s] [File1] [File2]\nTo Compare\n" COLOR_RESET, argv[0]);
		return 0;
	}

	auto GetFileRawName = [](const std::string &strFileName) -> std::string
	{
		//必须挨个查找，要么是最后一个左要么最后一个右
		auto szLastPathSeparator1 = strFileName.find_last_of('\\');
		auto szLastPathSeparator2 = strFileName.find_last_of('/');

		//如果找不到设置为0
		szLastPathSeparator1 = szLastPathSeparator1 != strFileName.npos ? szLastPathSeparator1 : 0;
		szLastPathSeparator2 = szLastPathSeparator2 != strFileName.npos ? szLastPathSeparator2 : 0;

		//选择最大的那个
		auto szLastPathSeparator = std::max(szLastPathSeparator1, szLastPathSeparator2);

		//没有，那么原始路径即为名称
		if (szLastPathSeparator == 0)
		{
			return strFileName;
		}

		//返回裁切视图
		return strFileName.substr(szLastPathSeparator + 1);//+1去掉最后一个分隔符
	};

	printf(COLOR_GREEN "File1: [%s]\n" COLOR_RESET, GetFileRawName(argv[1]).c_str());
	printf(COLOR_YELLOW "File2: [%s]\n" COLOR_RESET, GetFileRawName(argv[2]).c_str());

	auto ReadNBT = [](const std::string &strFileName, NBT_Type::Compound &cpdInput) -> bool
	{
		std::vector<uint8_t> vFileStream1{};
		if (!NBT_IO::ReadFile(strFileName, vFileStream1))
		{
			printf(COLOR_RED "Unable to read stream from file!\n" COLOR_RESET);
			return false;
		}

		//如果解压失败那么可能原先文件未压缩
		std::vector<uint8_t> vDataV7Stream{};
		if (!NBT_IO::DecompressDataNoThrow(vDataV7Stream, vFileStream1))
		{
			printf(COLOR_RED "Data may not be compressed, attempt to parse directly.\n" COLOR_RESET);
			vDataV7Stream = std::move(vFileStream1);//尝试以未压缩流处理，而不是失败
		}

		if (!NBT_Reader::ReadNBT(vDataV7Stream, 0, cpdInput))
		{
			printf(COLOR_RED "Unable to parse data from stream!\n" COLOR_RESET);
			return false;
		}

		return true;
	};

	NBT_Type::Compound cpdInput[2];
	bool b0 = ReadNBT(argv[1], cpdInput[0]);
	bool b1 = ReadNBT(argv[2], cpdInput[1]);

	if (!b0 || !b1)
	{
		printf(COLOR_RED "ReadNBT fail!\n" COLOR_RESET);
		return 0;
	}

	if (!cpdInput[0].HasCompound(MU8STR("")) ||
		!cpdInput[1].HasCompound(MU8STR("")))
	{
		printf(COLOR_RED "Root Compound not found!\n" COLOR_RESET);
		return 0;
	}

	auto tmp0 = std::move(cpdInput[0].GetCompound(MU8STR("")));
	cpdInput[0] = std::move(tmp0);

	auto tmp1 = std::move(cpdInput[1].GetCompound(MU8STR("")));
	cpdInput[1] = std::move(tmp1);


	std::vector<NBT_Compare::Report> listReports;
	bool bEqual = NBT_Compare::CompareDetails(cpdInput[0], cpdInput[1], listReports);
	MyAssert(bEqual == (cpdInput[0] == cpdInput[1]), "?");

	if (bEqual)
	{
		printf(COLOR_BLUE "Equal!\n" COLOR_RESET);
		return 0;
	}

	printf(COLOR_BLUE "No equal!\n" COLOR_RESET COLOR_BOLD "Diff Info:\n\n" COLOR_RESET);
	//详细信息输出
	
	setlocale(LC_ALL, ".UTF-8");
	for (auto &it : listReports)
	{
		printf(COLOR_CYAN "[%s]: " COLOR_RESET COLOR_BOLD "%s\n" COLOR_RESET "%s\n\n", NBT_Compare::GetDiffTypeInfo(it.enDiffInfo), it.strPath.c_str(), it.strDiffInfo.c_str());
	}
	setlocale(LC_ALL, "");

	printf(COLOR_BLUE "\nGen cmp file...\n" COLOR_RESET);

	//生成格式化文件方便文本查看
	//查找合法文件
	auto FindFileName = [](const std::string &strOldFileName, std::string &strNewFileName) -> bool
	{
		//找到后缀名
		size_t szPos = strOldFileName.find_last_of('.');

		//'.'前面的部分，不包含'.'
		std::string sNewFileName = strOldFileName.substr(0, szPos).append("Cmp");

		//唯一文件名
		strNewFileName = GenerateUniqueFilename(sNewFileName, ".txt");
		if (strNewFileName.empty())
		{
			printf(COLOR_RED "Unable to find a valid file name or lack of permission!\n" COLOR_RESET);
			return false;
		}

		return true;
	};

	std::string strCmpFileName[2];
	b0 = FindFileName(argv[1], strCmpFileName[0]);
	b1 = FindFileName(argv[2], strCmpFileName[1]);

	if (!b0 || !b1)
	{
		printf(COLOR_RED "FindFileName fail!\n" COLOR_RESET);
		return 0;
	}


	FILE *pFile[2];
	pFile[0] = fopen(strCmpFileName[0].c_str(), "wb");
	pFile[1] = fopen(strCmpFileName[1].c_str(), "wb");

	if (pFile[0] == NULL || pFile[1] == NULL)
	{
		fclose(pFile[0]);
		fclose(pFile[1]);
		return 0;
	}

	NBT_Helper::Print(cpdInput[0], NBT_Print{ pFile[0] });
	NBT_Helper::Print(cpdInput[1], NBT_Print{ pFile[1] });

	fclose(pFile[0]);
	fclose(pFile[1]);

	printf(COLOR_BLUE "Cmp file gen!\n" COLOR_RESET);
	return 0;
}


