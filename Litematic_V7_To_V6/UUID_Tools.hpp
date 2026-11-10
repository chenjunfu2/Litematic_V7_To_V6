#pragma once

#include <nbt_cpp/NBT_Node.hpp>
#include <util/MD5.hpp>

#include <format>
#include <ranges>
#include <utility>
#include <ranges>

struct UUID
{
	uint32_t data[4];
};

NBT_Type::IntArray UUID2IntArray(const UUID &uuid)
{
	return NBT_Type::IntArray
	{
		(NBT_Type::Int)uuid.data[0],
		(NBT_Type::Int)uuid.data[1],
		(NBT_Type::Int)uuid.data[2],
		(NBT_Type::Int)uuid.data[3],
	};
}

bool IntArray2UUID(const NBT_Type::IntArray &iaUuidList, UUID &uuid)
{
	if (iaUuidList.size() != 4)
	{
		return false;
	}

	uuid.data[0] = (uint32_t)iaUuidList[0];
	uuid.data[1] = (uint32_t)iaUuidList[1];
	uuid.data[2] = (uint32_t)iaUuidList[2];
	uuid.data[3] = (uint32_t)iaUuidList[3];

	return true;
}

//等价java.util.UUID.nameUUIDFromBytes
UUID NameUuidFromBytes(const void *data, size_t len)
{
	uint8_t digest[16];
	MD5::FastMd5((const uint8_t *)data, len, digest);//这东西实现搞死我了
	
	digest[6] = (uint8_t)((digest[6] & 0x0F) | 0x30);//version = 3
	digest[8] = (uint8_t)((digest[8] & 0x3F) | 0x80);//variant = IETF

	//直接按大端把 16 字节拆成 4 个 int32（等价于原 msb/lsb 再拆分）
	auto rd32 = [&](int i) -> uint32_t
	{
		return 
			((uint32_t)(digest[i + 0]) << 24) |
			((uint32_t)(digest[i + 1]) << 16) |
			((uint32_t)(digest[i + 2]) <<  8) |
			((uint32_t)(digest[i + 3]) <<  0);
	};

	return UUID
	{
		rd32( 0), //msb >> 32
		rd32( 4), //msb
		rd32( 8), //lsb >> 32
		rd32(12), //lsb
	};
}

NBT_Type::String Uuid2String(const UUID &uuid)
{
	uint32_t time_low = uuid.data[0];
	uint16_t time_mid = ((uint32_t)uuid.data[1] >> 16) & 0xFFFFu;
	uint16_t time_hi_and_version = (uint32_t)uuid.data[1] & 0xFFFFu;
	uint16_t clock_seq = ((uint32_t)uuid.data[2] >> 16) & 0xFFFFu;
	uint64_t node = (((uint64_t)((uint32_t)uuid.data[2] & 0xFFFFu) << 32) | (uint64_t)(uint32_t)uuid.data[3]) & 0xFFFFFFFFFFFFull;

	return (NBT_Type::String)std::format(
		"{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
		time_low, time_mid, time_hi_and_version, clock_seq, node
	);
}


template<typename C>
constexpr bool String2UuidImpl(UUID &uuid, const C *pStrUuid, size_t szStringLen)
{
	if (szStringLen != (8 + 1 + (4 + 1) * 3 + 12))
	{
		return false;
	}

	//首先验证连字符
	//8-4-4-4-12
	//8 13 18 23
	if (pStrUuid[ 8] != '-' ||
		pStrUuid[13] != '-' ||
		pStrUuid[18] != '-' ||
		pStrUuid[23] != '-')
	{
		return false;
	}

	static const auto GetCharByte =
	[](uint32_t &data, MUTF8_Char_Type ch, MUTF8_Char_Type cl) constexpr -> bool
	{
		static constexpr auto charMap = //字符映射值
		[]() constexpr -> auto
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

	static constexpr auto indexMap = //索引映射
	[]() constexpr -> auto
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
		if (!GetCharByte(uuid.data[i / 4], pStrUuid[indexMap[i * 2 + 0]], pStrUuid[indexMap[i * 2 + 1]]))
		{
			return false;
		}
	}

	return true;
}

bool String2Uuid(NBT_Type::String &strUuid, NBT_Type::IntArray &iaUuidList)
{
	if (strUuid.size() != (8 + 1 + (4 + 1) * 3 + 12))
	{
		return false;
	}

	UUID uuid{};
	if (!String2UuidImpl(uuid, strUuid.data(), strUuid.size()))
	{
		return false;
	}
	
	//转换到nbt数组
	UUID2IntArray(uuid, iaUuidList);
	return true;
}

template<MUTF8_Tool_Internal::StringLiteral strUUID>
constexpr UUID String2Uuid(void)//编译期生成使用
{
	UUID uuid{};
	if (!String2UuidImpl(uuid, strUUID.data(), strUUID.size()))
	{
		throw "Invalid UUID string format";
	}
	
	return uuid;
}
