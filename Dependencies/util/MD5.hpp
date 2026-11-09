#pragma once
#include <type_traits>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <utility>
#include <array>
#include <bit>

class MD5
{
	MD5(void) = delete;
	~MD5(void) = delete;
public:
	static constexpr uint32_t KVal[64] =
	{
		0XD76AA478u, 0XE8C7B756u, 0X242070DBu, 0XC1BDCEEEu,
		0XF57C0FAFu, 0X4787C62Au, 0XA8304613u, 0XFD469501u,
		0X698098D8u, 0X8B44F7AFu, 0XFFFF5BB1u, 0X895CD7BEu,
		0X6B901122u, 0XFD987193u, 0XA679438Eu, 0X49B40821u,
		0XF61E2562u, 0XC040B340u, 0X265E5A51u, 0XE9B6C7AAu,
		0XD62F105Du, 0X02441453u, 0XD8A1E681u, 0XE7D3FBC8u,
		0X21E1CDE6u, 0XC33707D6u, 0XF4D50D87u, 0X455A14EDu,
		0XA9E3E905u, 0XFCEFA3F8u, 0X676F02D9u, 0X8D2A4C8Au,
		0XFFFA3942u, 0X8771F681u, 0X6D9D6122u, 0XFDE5380Cu,
		0XA4BEEA44u, 0X4BDECFA9u, 0XF6BB4B60u, 0XBEBFBC70u,
		0X289B7EC6u, 0XEAA127FAu, 0XD4EF3085u, 0X04881D05u,
		0XD9D4D039u, 0XE6DB99E5u, 0X1FA27CF8u, 0XC4AC5665u,
		0XF4292244u, 0X432AFF97u, 0XAB9423A7u, 0XFC93A039u,
		0X655B59C3u, 0X8F0CCC92u, 0XFFEFF47Du, 0X85845DD1u,
		0X6FA87E4Fu, 0XFE2CE6E0u, 0XA3014314u, 0X4E0811A1u,
		0XF7537E82u, 0XBD3AF235u, 0X2AD7D2BBu, 0XEB86D391u
	};

	static constexpr uint8_t SVal[64] =
	{
		 7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
		 5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
		 4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
		 6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
	};

	template<size_t i>
	static inline consteval uint32_t K(void)
	{
		return KVal[i];
	}

	template<size_t i>
	static inline consteval uint8_t S(void)
	{
		return SVal[i];
	}

	static inline constexpr uint32_t ROTL32(uint32_t v, uint8_t c)
	{
		return (v << c) | (v >> (32 - c));
	};

	template<size_t i>
	static inline constexpr uint32_t Md5RFSwitch(const uint32_t &b, const uint32_t &c, const uint32_t &d)
	{
		if constexpr (i >= 0 && i < 16)
		{
			return (b & c) | (~b & d);
		}
		else if constexpr (i >= 16 && i < 32)
		{
			return (d & b) | (~d & c);
		}
		else if constexpr (i >= 32 && i < 48)
		{
			return b ^ c ^ d;
		}
		else if constexpr (i >= 48 && i < 64)
		{
			return c ^ (b | ~d);
		}
		else
		{
			static_assert(false, "?");
		}
	}

	template<size_t i>
	static inline consteval uint32_t Md5MISwitch(void)
	{
		if constexpr (i >= 0 && i < 16)
		{
			return i;
		}
		else if constexpr (i >= 16 && i < 32)
		{
			return (5 * i + 1) % 16;
		}
		else if constexpr (i >= 32 && i < 48)
		{
			return (3 * i + 5) % 16;
		}
		else if constexpr (i >= 48 && i < 64)
		{
			return (7 * i) % 16;
		}
		else
		{
			static_assert(false, "?");
		}
	}


	template<size_t i>
	static inline constexpr void Md5CompressImpl(uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d, const uint32_t(&M)[16])
	{
		uint32_t old_b = b;
		b += ROTL32(a + Md5RFSwitch<i>(b, c, d) + K<i>() + M[Md5MISwitch<i>()], S<i>());
		a = d;
		d = c;
		c = old_b;
	}

	template<std::size_t... i>
	static inline constexpr void MakeMd5Compress(uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d, const uint32_t(&M)[16], std::index_sequence<i...>)
	{
		(Md5CompressImpl<i>(a, b, c, d, M), ...);
	}

	static inline constexpr void Md5Compress(uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d, const uint32_t(&M)[16])
	{
		uint32_t calc_a = a;
		uint32_t calc_b = b;
		uint32_t calc_c = c;
		uint32_t calc_d = d;

		MakeMd5Compress(calc_a, calc_b, calc_c, calc_d, M, std::make_index_sequence<64>{});

		a += calc_a;
		b += calc_b;
		c += calc_c;
		d += calc_d;
	}


	template<size_t i, typename T>
	requires(sizeof(T) == 1)
	static inline constexpr uint32_t MakeM(T pData[64])
	{
		static_assert(i >= 0 && i < 16, "?");

		return
			((uint32_t)(uint8_t)pData[i * 4 + 0] <<  0) |
			((uint32_t)(uint8_t)pData[i * 4 + 1] <<  8) |
			((uint32_t)(uint8_t)pData[i * 4 + 2] << 16) |
			((uint32_t)(uint8_t)pData[i * 4 + 3] << 24);
	}

	template<typename T, std::size_t... i>
	requires(sizeof(T) == 1)
	static inline constexpr void MakeMArr(uint32_t(&M)[16], T pData[64], std::index_sequence<i...>)
	{
		((M[i] = MakeM<i>(pData)), ...);
	}

	template<typename T>
	requires(sizeof(T) == 1)
	static inline constexpr void Md5Message(uint32_t(&M)[16], T pData[64])
	{
		MakeMArr(M, pData, std::make_index_sequence<16>{});
	}

	template<typename T>
	requires(sizeof(T) == 1)
	static inline constexpr void Md5Step(T pData[64], uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d)
	{
		uint32_t M[16];
		Md5Message(M, pData);
		Md5Compress(a, b, c, d, M);
	}

	static inline constexpr size_t Md5PaddedLength(size_t len)//对齐到64
	{
		return (((len + 1ull + 8ull) + 63ull) / 64ull) * 64ull;
	}

	template<typename T>
	requires(sizeof(T) == 1)
	static inline constexpr void Md5AppendPaddingByte(T pData[1])
	{
		pData[0] = 0x80;
	}

	template<typename T>
	requires(sizeof(T) == 1)
	static inline constexpr void Md5AppendBitLength(T pData[8], uint64_t len)
	{
		len *= 8;//to bit len

		pData[0] = std::bit_cast<T>((uint8_t)(len >> (8 * 0)));
		pData[1] = std::bit_cast<T>((uint8_t)(len >> (8 * 1)));
		pData[2] = std::bit_cast<T>((uint8_t)(len >> (8 * 2)));
		pData[3] = std::bit_cast<T>((uint8_t)(len >> (8 * 3)));
		pData[4] = std::bit_cast<T>((uint8_t)(len >> (8 * 4)));
		pData[5] = std::bit_cast<T>((uint8_t)(len >> (8 * 5)));
		pData[6] = std::bit_cast<T>((uint8_t)(len >> (8 * 6)));
		pData[7] = std::bit_cast<T>((uint8_t)(len >> (8 * 7)));
	}


	template<typename T>
	requires(sizeof(T) == 1)
	static inline constexpr void Md5OutputImpl(T md5[4], const uint32_t &v)
	{
		md5[0] = std::bit_cast<T>((uint8_t)(v >> (8 * 0)));
		md5[1] = std::bit_cast<T>((uint8_t)(v >> (8 * 1)));
		md5[2] = std::bit_cast<T>((uint8_t)(v >> (8 * 2)));
		md5[3] = std::bit_cast<T>((uint8_t)(v >> (8 * 3)));
	}

	template<typename T>
	requires(sizeof(T) == 1)
	static inline constexpr void Md5Output(T md5[16], const uint32_t &a, const uint32_t &b, const uint32_t &c, const uint32_t &d)
	{
		Md5OutputImpl(&md5[0], a);
		Md5OutputImpl(&md5[4], b);
		Md5OutputImpl(&md5[8], c);
		Md5OutputImpl(&md5[12], d);
	}

	static inline constexpr void Md5Init(uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d)
	{
		a = 0X67452301u;
		b = 0XEFCDAB89u;
		c = 0X98BADCFEu;
		d = 0X10325476u;
	}

public:
	//512bit = 64byte
	template<typename T>
	requires(sizeof(T) == 1)
	static inline constexpr void FastMd5(const T *data, size_t len, uint8_t out[16])
	{
		uint32_t a, b, c, d;
		Md5Init(a, b, c, d);

		size_t fullLen = len - len % 64;
		for (size_t off = 0; off < fullLen; off += 64)
		{
			Md5Step(&data[off], a, b, c, d);
		}

		// 剩余数据
		size_t tailLen = len - fullLen;
		size_t tailBlocks = (tailLen < 56) ? 64 : 128;

		uint8_t tail[128] = { 0 };//全0初始化，省略后续填充
		if (std::is_constant_evaluated())//编译期不走memcpy
		{
			for (size_t i = 0; i < tailLen; ++i)
			{
				tail[i] = data[fullLen + i];
			}
		}
		else
		{
			memcpy(&tail[0], &data[fullLen], tailLen);
		}

		Md5AppendPaddingByte(&tail[tailLen]);
		Md5AppendBitLength(&tail[tailBlocks - 8], len);
		Md5Step(&tail[0], a, b, c, d);
		if (tailBlocks > 64)
		{
			Md5Step(&tail[64], a, b, c, d);
		}

		Md5Output(out, a, b, c, d);
	}

	template<typename T, size_t N>
	requires(sizeof(T) == 1)
	static inline constexpr void FastMd5(const T(&data)[N], uint8_t out[16])
	{
		FastMd5(+data, N, out);
	}

	template<typename T, size_t N>
	requires(sizeof(T) == 1)
	static inline constexpr void FastMd5(const std::array<T, N> data, uint8_t out[16])
	{
		FastMd5(data.data(), data.size(), out);
	}
};

namespace MD5Test
{
	template <size_t N>
	struct FixedString : public std::array<char, N>
	{
		using Super = std::array<char, N>;
		constexpr FixedString(const char(&str)[N]) noexcept
			: Super(std::to_array(str))
		{}
	};

	template<FixedString str>
	consteval std::array<uint8_t, 16> HexStringToMd5Bytes(void)
	{
		std::array<uint8_t, 16> md5{};

		if (str.size() < 32)
		{
			return md5;
		}

		auto ToHex =
			[](char ch, char cl) consteval -> uint8_t
		{
			auto ToHexImpl =
				[](char c) consteval -> uint8_t
			{
				if (c >= '0' && c <= '9')
				{
					return (uint8_t)(c - '0');
				}
				else if (c >= 'A' && c <= 'F')
				{
					return (uint8_t)(c - 'A' + 0x0A);
				}
				else if (c >= 'a' && c <= 'f')
				{
					return (uint8_t)(c - 'a' + 0x0a);
				}
				else
				{
					return (uint8_t)0;
				}
			};

			return (ToHexImpl(ch) << 4) | (ToHexImpl(cl) & 0x0Fu);
		};

		for (size_t i = 0; i < 32; i += 2)
		{
			md5[i / 2] = ToHex(str[i], str[i + 1]);
		}

		return md5;
	}

	template<FixedString str, FixedString md5>
	consteval bool VerifyMd5(void)
	{
		std::array<uint8_t, 16> out{};
		MD5::FastMd5(str.data(), str.size() - 1, out.data());
		return out == HexStringToMd5Bytes<md5>();
	}

	static_assert(VerifyMd5<"", "d41d8cd98f00b204e9800998ecf8427e">(), "fail0");
	static_assert(VerifyMd5<"a", "0cc175b9c0f1b6a831c399e269772661">(), "fail1");
	static_assert(VerifyMd5<"abc", "900150983cd24fb0d6963f7d28e17f72">(), "fail2");
	static_assert(VerifyMd5<"message digest", "f96b697d7cb7938d525a2f31aaf161d0">(), "fail3");
	static_assert(VerifyMd5<"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b">(), "fail4");
}
