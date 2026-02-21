#include <nbt_cpp/NBT_All.hpp>
#include <util/CodeTimer.hpp>
#include <util/CPP_Helper.h>

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
	enum class DiffInfo
	{
		NoDiff,
		DiffTag,
		DiffVal,
		DiffLen,
		DiffKey,
	};

	struct Report
	{
		NBT_Type::String strPath;
		DiffInfo enDiffInfo;
		std::string strDiffInfo;
	};


private:
	NBT_Type::String ConnectionPath(std::vector<NBT_Type::String> &listNbtPath)
	{
		NBT_Type::String tmp;
		for (auto &it : listNbtPath)
		{
			tmp.append(it);
		}

		return tmp;
	}

	template<typename T>
	std::string GenInfo(const T &tL, const T &tR)
	{
		std::string strInfo;
		if constexpr (std::is_same_v<T, NBT_Type::String>)
		{
			strInfo += "\"";
			strInfo += tL.ToCharTypeUTF8();
			strInfo += "\" != \"";
			strInfo += tR.ToCharTypeUTF8();
			strInfo += "\"";
		}
		else if constexpr (std::is_same_v<T, std::string>)
		{
			strInfo += tL;
			strInfo += " != ";
			strInfo += tR;
		}
		else
		{
			strInfo += "[";
			strInfo += std::to_string(tL);
			strInfo += "] != [";
			strInfo += std::to_string(tR);
			strInfo += "]";
		}
		
		return strInfo;
	}

private:

	bool CompareCompoundType(const NBT_Node &nodeLeft, const NBT_Node &nodeRight, std::vector<Report> &listReports, std::vector<NBT_Type::String> &listNbtPath)
	{






	}

	bool CompareListType(const NBT_Type::List &listLeft, const NBT_Type::List &listRight, std::vector<Report> &listReports, std::vector<NBT_Type::String> &listNbtPath)
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
				GenInfo(NBT_Type::GetTypeName(listLeft.GetTag()), NBT_Type::GetTypeName(listRight.GetTag())));
			return false;//成员类型不同直接不用比较
		}

		//收集列表所有成员，然后选出相同部分



		return bRet;
	}

	bool CompareStringType(const NBT_Type::String &strLeft, const NBT_Type::String &strRight, std::vector<Report> &listReports, std::vector<NBT_Type::String> &listNbtPath)
	{
		if (strLeft.size() != strRight.size())
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffLen,
				GenInfo(strLeft.size(), strRight.size()));
			return false;
		}

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
	bool CompareArrayType(const T &arrayLeft, const T &arrayRight, std::vector<Report> &listReports, std::vector<NBT_Type::String> &listNbtPath)
	{
		if (arrayLeft.size() != arrayRight.size())
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffLen,
				GenInfo(arrayLeft.size(), arrayRight.size()));
			return false;
		}

		if (arrayLeft != arrayRight)
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffVal,
				GenInfo(NBT_Helper::Serialize(arrayLeft), NBT_Helper::Serialize(arrayRight)));
			return false;
		}

		return true;
	}

	template<typename T>
	bool CompareBuiltinType(const T &builtinLeft, const T &builtinRight, std::vector<Report> &listReports, std::vector<NBT_Type::String> &listNbtPath)
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
	bool CompareDetailsImpl(std::conditional_t<bRoot, const NBT_Node_View<true> &, const NBT_Node &> nodeLeft, std::conditional_t<bRoot, const NBT_Node_View<true> &, const NBT_Node &> nodeRight, std::vector<Report> &listReports, std::vector<NBT_Type::String> &listNbtPath)
	{
		NBT_TAG tagType = nodeLeft.GetTag();
		if (tagType != nodeRight.GetTag())
		{
			listReports.emplace_back(
				ConnectionPath(listNbtPath),
				DiffInfo::DiffTag,
				GenInfo(NBT_Type::GetTypeName(nodeLeft.GetTag()), NBT_Type::GetTypeName(nodeRight.GetTag())));
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
	bool CompareDetails(const NBT_Node_View<true> nodeLeft, const NBT_Node_View<true> nodeRight, std::vector<Report> &listReports)
	{
		std::vector<NBT_Type::String> listNbtPath{};
		return CompareDetailsImpl<true>(nodeLeft, nodeRight, listReports, listNbtPath);
	}

};



int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Use:\n>[%s] [File1] [File2]\nTo Compare\n", argv[0]);
		return 0;
	}

	auto ReadNBT = [](const char *pFileName, NBT_Type::Compound &cpdInput) -> bool
	{
		std::vector<uint8_t> vFileStream1{};
		if (!NBT_IO::ReadFile(pFileName, vFileStream1))
		{
			printf("Unable to read stream from file!\n");
			return false;
		}

		//如果解压失败那么可能原先文件未压缩
		std::vector<uint8_t> vDataV7Stream{};
		if (!NBT_IO::DecompressDataNoThrow(vDataV7Stream, vFileStream1))
		{
			printf("Data may not be compressed, attempt to parse directly.\n");
			vDataV7Stream = std::move(vFileStream1);//尝试以未压缩流处理，而不是失败
		}

		if (!NBT_Reader::ReadNBT(vDataV7Stream, 0, cpdInput))
		{
			printf("Unable to parse data from stream!\n");
			return false;
		}

		return true;
	};

	NBT_Type::Compound cpdInput[2];
	bool b0 = ReadNBT(argv[1], cpdInput[0]);
	bool b1 = ReadNBT(argv[2], cpdInput[1]);

	if (!b0 || !b1)
	{
		printf("ReadNBT fail!\n");
		return 0;
	}

	if (!cpdInput[0].HasCompound(MU8STR("")) ||
		!cpdInput[1].HasCompound(MU8STR("")))
	{
		printf("Root Compound not found!\n");
		return 0;
	}

	auto tmp0 = std::move(cpdInput[0].GetCompound(MU8STR("")));
	cpdInput[0] = std::move(tmp0);

	auto tmp1 = std::move(cpdInput[1].GetCompound(MU8STR("")));
	cpdInput[1] = std::move(tmp1);

	if (cpdInput[0] == cpdInput[1])
	{
		printf("Equal!\n");
		return 0;
	}

	printf("No equal!\n");
	//详细比较
	//CompareDetails(cpdInput[0], cpdInput[1]);


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
			printf("Unable to find a valid file name or lack of permission!\n");
			return false;
		}

		return true;
	};

	std::string strCmpFileName[2];
	b0 = FindFileName(argv[1], strCmpFileName[0]);
	b1 = FindFileName(argv[2], strCmpFileName[1]);

	if (!b0 || !b1)
	{
		printf("FindFileName fail!\n");
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

	printf("Cmp file gen!\n");
	return 0;
}


