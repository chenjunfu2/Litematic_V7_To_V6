#include <nbt_cpp/NBT_All.hpp>

#include <vector>
#include <string>
#include <stdio.h>
#include <limits>
#include <format>

/// @brief 流式打印Visitor，随Scanner按二进制流实际顺序直接输出NBT到标准输出
/// @note 不构建内存树，不排序，完全按照二进制流中的条目顺序输出
class PrintVisitor
{
public:
	using ResultControl = NBT_Visitor::ResultControl;
	using NestingControl = NBT_Visitor::NestingControl;

private:
	struct Frame
	{
		enum Type : uint8_t { Compound, List };
		Type type;
		bool firstEntry = true;
		bool bracketOpened = false;	// 开括号是否已打印（空容器推迟到结束时打印 {} 或 []）
	};

	std::vector<Frame> frames;
	FILE *out;
	std::string indentStr;

	void printIndent()
	{
		for (size_t i = 0; i < frames.size(); ++i)
			fputs(indentStr.c_str(), out);
	}

	// 在容器条目开始时调用，负责逗号和缩进
	void beginEntry()
	{
		if (frames.empty()) return;
		auto &top = frames.back();
		if (!top.firstEntry)
		{
			fputs(",\n", out);
			printIndent();
		}
		else
		{
			top.firstEntry = false;
			fputs("\n", out);
			printIndent();
		}
	}

	// 确保非空容器的开括号已打印（延迟到首个条目出现时才打印，以实现空容器 {}[] 同行）
	void ensureBracketOpen()
	{
		if (frames.empty()) return;
		auto &top = frames.back();
		if (top.bracketOpened) return;
		top.bracketOpened = true;

		fputs("\n", out);
		for (size_t i = 0; i + 1 < frames.size(); ++i)
			fputs(indentStr.c_str(), out);

		if (top.type == Frame::Compound)
			fputc('{', out);
		else
			fputc('[', out);
	}

	// 关闭容器，负责换行和缩进到闭合括号层级
	void closeContainer(const char *closeBracket)
	{
		if (frames.empty()) return;
		auto &top = frames.back();
		if (!top.bracketOpened)
		{
			// 空容器，直接打印 {} 或 []
			fputs(top.type == Frame::Compound ? "{}" : "[]", out);
		}
		else
		{
			if (!top.firstEntry)
			{
				fputs("\n", out);
				for (size_t i = 0; i + 1 < frames.size(); ++i)
					fputs(indentStr.c_str(), out);
			}
			fputs(closeBracket, out);
		}
		frames.pop_back();
	}

public:
	explicit PrintVisitor(FILE *f = stdout, std::string indent = "    ")
		: out(f), indentStr(std::move(indent)) {}

	// === 数值类型 ===
	template<typename T> requires(NBT_Type::IsNumericType_V<T>)
	ResultControl VisitNumericResult(T val)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			auto s = std::format("{:.{}g}", val, std::numeric_limits<T>::max_digits10);
			fputs(s.c_str(), out);
		}
		else
		{
			auto s = std::format("{:d}", val);
			fputs(s.c_str(), out);
		}

		if constexpr (std::is_same_v<T, NBT_Type::Byte>)       fputs("B", out);
		else if constexpr (std::is_same_v<T, NBT_Type::Short>) fputs("S", out);
		else if constexpr (std::is_same_v<T, NBT_Type::Int>)   fputs("I", out);
		else if constexpr (std::is_same_v<T, NBT_Type::Long>)  fputs("L", out);
		else if constexpr (std::is_same_v<T, NBT_Type::Float>) fputs("F", out);
		else if constexpr (std::is_same_v<T, NBT_Type::Double>)fputs("D", out);

		return ResultControl::Continue;
	}

	// === 数组类型（ByteArray / IntArray / LongArray）===
	template<typename T> requires(NBT_Type::IsArrayType_V<T> && !std::is_reference_v<T>)
	ResultControl VisitArrayResult(T &&arr)
	{
		if constexpr (std::is_same_v<std::decay_t<T>, NBT_Type::ByteArray>)
			fputs("[B;", out);
		else if constexpr (std::is_same_v<std::decay_t<T>, NBT_Type::IntArray>)
			fputs("[I;", out);
		else if constexpr (std::is_same_v<std::decay_t<T>, NBT_Type::LongArray>)
			fputs("[L;", out);

		bool first = true;
		for (const auto &elem : arr)
		{
			if (first) first = false;
			else fputs(",", out);

			auto s = std::format("{:d}", elem);
			fputs(s.c_str(), out);
		}
		fputs("]", out);
		return ResultControl::Continue;
	}

	// === 字符串 ===
	ResultControl VisitStringResult(NBT_Type::String &&str)
	{
		fputc('"', out);
		fputs(str.ToCharTypeUTF8().c_str(), out);
		fputc('"', out);
		return ResultControl::Continue;
	}

	// === End ===
	ResultControl VisitEndResult()
	{
		fputs("[End]", out);
		return ResultControl::Continue;
	}

	// === Compound ===
	ResultControl VisitCompoundBegin()
	{
		frames.push_back({Frame::Compound, true, false});
		return ResultControl::Continue;
	}

	NestingControl VisitCompoundNextEntryType(NBT_TAG tag)
	{
		(void)tag;
		ensureBracketOpen();
		beginEntry();
		return NestingControl::Enter;
	}

	NestingControl VisitCompoundEntryBegin(NBT_TAG tag, NBT_Type::String &&name)
	{
		(void)tag;
		fputc('"', out);
		fputs(name.ToCharTypeUTF8().c_str(), out);
		fputs("\":", out);
		return NestingControl::Enter;
	}

	ResultControl VisitCompoundEntryEnd(NBT_TAG tag, NBT_Type::String &&name)
	{
		(void)tag;
		(void)name;
		return ResultControl::Continue;
	}

	ResultControl VisitCompoundEnd()
	{
		closeContainer("}");
		return ResultControl::Continue;
	}

	// === List ===
	ResultControl VisitListBegin(NBT_TAG tag, size_t len)
	{
		(void)tag;
		(void)len;
		frames.push_back({Frame::List, true, false});
		return ResultControl::Continue;
	}

	NestingControl VisitListElementBegin(NBT_TAG tag, size_t idx)
	{
		(void)tag;
		(void)idx;
		ensureBracketOpen();
		beginEntry();
		return NestingControl::Enter;
	}

	ResultControl VisitListElementEnd(NBT_TAG tag, size_t idx)
	{
		(void)tag;
		(void)idx;
		return ResultControl::Continue;
	}

	ResultControl VisitListEnd()
	{
		closeContainer("]");
		return ResultControl::Continue;
	}

	// === 根部回调 ===
	void VisitBegin()
	{
		fputc('{', out);
		frames.push_back({Frame::Compound, true, true});
	}

	void VisitEnd()
	{
		closeContainer("}");
		fputc('\n', out);
	}

	// === 错误处理 ===
	template<typename... Args>
	void VisitError(NBT_Print_Level lvl, const std::format_string<Args...> fmt, Args&&... args) noexcept
	{
		(void)lvl;
		try
		{
			auto s = std::format(std::move(fmt), std::forward<Args>(args)...);
			fputs(s.c_str(), stderr);
		}
		catch (...)
		{
			fputs("Error formatting error message\n", stderr);
		}
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <NBT file path>\n", argv[0]);
		printf("Stream-print NBT data directly from binary stream to stdout.\n");
		return(0);
	}

	// 读取文件
	std::vector<uint8_t> vFileData;
	if (!NBT_IO::ReadFile(argv[1], vFileData))
	{
		fprintf(stderr, "Error: Cannot read file [%s]\n", argv[1]);
		return(1);
	}

	// 尝试GZip解压，失败则视作未压缩数据
	std::vector<uint8_t> vNbtData;
	if (!NBT_IO::DecompressDataNoThrow(vNbtData, vFileData))
	{
		vNbtData = std::move(vFileData);
	}

	// 创建流式打印Visitor并扫描
	PrintVisitor visitor(stdout, "    ");

	if (!NBT_Scanner::ScanNBT(vNbtData, 0, visitor))
	{
		fprintf(stderr, "Error: NBT scan failed\n");
		return(1);
	}

	return(0);
}
