#include "Litematic_V7_To_V6.h"

#include <stdint.h>
#include "jni/include/jni.h"


/// @brief JNI 输入流适配器，用于从 jbyteArray 读取数据
class JNIInputStream
{
private:
	JNIEnv *env;
	jbyteArray data;
	jbyte *buffer;
	size_t size;
	size_t position;

public:
	/// @brief 构造函数，从 jbyteArray 创建输入流
	/// @param env JNI 环境指针
	/// @param input Java 字节数组
	JNIInputStream(JNIEnv *env, jbyteArray input)
		: env(env), data(input), buffer(nullptr), size(0), position(0)
	{
		if (input == nullptr)
		{
			throw std::runtime_error("Input jbyteArray is null");
		}

		size = env->GetArrayLength(input);
		buffer = env->GetByteArrayElements(input, nullptr);

		if (buffer == nullptr)
		{
			throw std::runtime_error("Failed to get byte array elements");
		}
	}

	/// @brief 析构函数，自动释放 JNI 资源
	~JNIInputStream()
	{
		if (buffer != nullptr)
		{
			env->ReleaseByteArrayElements(data, buffer, JNI_ABORT);
			buffer = nullptr;
		}
	}

	// 禁止拷贝和移动
	JNIInputStream(const JNIInputStream &) = delete;
	JNIInputStream(JNIInputStream &&) = delete;
	JNIInputStream &operator=(const JNIInputStream &) = delete;
	JNIInputStream &operator=(JNIInputStream &&) = delete;

	/// @brief 下标访问运算符
	const uint8_t &operator[](size_t index) const noexcept
	{
		return buffer[index];
	}

	/// @brief 获取下一个字节并推进读取位置
	const uint8_t &GetNext() noexcept
	{
		return buffer[position++];
	}

	/// @brief 从流中读取一段数据
	void GetRange(void *pDest, size_t szSize) noexcept
	{
		memcpy(pDest, &buffer[position], szSize);
		position += szSize;
	}

	/// @brief 回退一个字节
	void UnGet() noexcept
	{
		if (position > 0)
		{
			--position;
		}
	}

	/// @brief 获取当前读取位置的指针
	const uint8_t *CurData() const noexcept
	{
		return (uint8_t *)&buffer[position];
	}

	/// @brief 向后推进读取
	size_t AddIndex(size_t szSize) noexcept
	{
		position += szSize;
		return position;
	}

	/// @brief 向前撤销读取
	size_t SubIndex(size_t szSize) noexcept
	{
		position -= szSize;
		return position;
	}

	/// @brief 检查是否已到达流末尾
	bool IsEnd() const noexcept
	{
		return position >= size;
	}

	/// @brief 获取流的总大小
	size_t Size() const noexcept
	{
		return size;
	}

	/// @brief 检查是否还有足够的数据
	bool HasAvailData(size_t szSize) const noexcept
	{
		return (size - position) >= szSize;
	}

	/// @brief 重置流读取位置
	void Reset() noexcept
	{
		position = 0;
	}

	/// @brief 获取底层数据的起始指针
	const uint8_t *BaseData() const noexcept
	{
		return (const uint8_t *)buffer;
	}

	/// @brief 获取当前读取位置（只读）
	size_t Index() const noexcept
	{
		return position;
	}

	/// @brief 获取当前读取位置（可写）
	size_t &Index() noexcept
	{
		return position;
	}
};

/// @brief JNI 输出流适配器，用于将数据写入到 jbyteArray
/// @note 符合你定义的 DefaultOutputStream 鸭子类型接口
class JNIOutputStream
{
private:
	JNIEnv *env;
	std::vector<uint8_t> buffer;  // 先写入到 vector，最后再转为 jbyteArray

public:
	using StreamType = jbyteArray;
	using ValueType = uint8_t;

	/// @brief 构造函数
	JNIOutputStream(JNIEnv *env, size_t szReserve = 1024) : env(env)
	{
		//初始预留空间
		buffer.reserve(szReserve);
	}

	// 禁止拷贝和移动
	JNIOutputStream(const JNIOutputStream &) = delete;
	JNIOutputStream(JNIOutputStream &&) = delete;
	JNIOutputStream &operator=(const JNIOutputStream &) = delete;
	JNIOutputStream &operator=(JNIOutputStream &&) = delete;

	/// @brief 下标访问运算符（只读）
	const ValueType &operator[](size_t index) const noexcept
	{
		return buffer[index];
	}

	/// @brief 向流中写入单个值
	template<typename V>
	requires(std::is_constructible_v<ValueType, V &&>)
		void PutOnce(V &&c)
	{
		buffer.push_back(static_cast<ValueType>(std::forward<V>(c)));
	}

	/// @brief 向流中写入一段数据
	void PutRange(const ValueType *pData, size_t szSize)
	{
		size_t currentSize = buffer.size();
		buffer.resize(currentSize + szSize);
		memcpy(&buffer[currentSize], pData, szSize);
	}

	/// @brief 预分配额外容量
	void AddReserve(size_t szAddSize)
	{
		buffer.reserve(buffer.size() + szAddSize);
	}

	/// @brief 删除最后一个写入的字节
	void UnPut() noexcept
	{
		if (!buffer.empty())
		{
			buffer.pop_back();
		}
	}

	/// @brief 获取当前字节流大小
	size_t Size() const noexcept
	{
		return buffer.size();
	}

	/// @brief 重置流，清空所有数据
	void Reset() noexcept
	{
		buffer.clear();
	}

	/// @brief 获取底层数据的常量引用
	const std::vector<uint8_t> &Data() const noexcept
	{
		return buffer;
	}

	/// @brief 获取底层数据的非常量引用
	std::vector<uint8_t> &Data() noexcept
	{
		return buffer;
	}

	/// @brief 创建 jbyteArray 并转移数据所有权
	/// @return 新创建的 jbyteArray，需要调用者用 DeleteLocalRef 释放
	jbyteArray ToJByteArray()
	{
		if (buffer.size() > INT64_MAX)
		{
			throw std::runtime_error("JNI byte array size exceeds maximum allowed limit: " +
				std::to_string(buffer.size()) + " bytes (max: " +
				std::to_string(INT64_MAX) + " bytes)");
		}

		jsize len = (jsize)buffer.size();
		jbyteArray result = env->NewByteArray(len);

		if (result != nullptr && len > 0)
		{
			env->SetByteArrayRegion(result, 0, len, (const jbyte *)buffer.data());
		}

		return result;
	}
};

extern "C"
{
	JNIEXPORT jint JNICALL Java_getVersion(JNIEnv *env, jclass clazz)//jobject obj
	{
		return env->GetVersion();
	}

	JNIEXPORT jbyteArray JNICALL Java_litematic_Convert_V7_To_V6(JNIEnv * env, jclass clazz, jbyteArray input)//jobject obj
	{
		if (input == nullptr)
		{
			return nullptr;
		}
	
		try
		{
			// 创建输入流
			NBT_Type::Compound cpdTmpV7Input{};
			size_t szV7StreamSize = 0;
			{
				JNIInputStream inputV7Stream(env, input);
				szV7StreamSize = inputV7Stream.Size();
				if (!NBT_Reader::ReadNBT(inputV7Stream, cpdTmpV7Input, 512, NBT_Print{ NULL,NULL,NULL }))
				{
					throw std::runtime_error("Unable to parse data from stream!");
				}
			}

			// 创建输出流
			JNIOutputStream outputV6Stream(env, szV7StreamSize);
			{
				//转换数据
				NBT_Type::Compound cpdV6Output{};
				NBT_Type::Compound cpdV7Input = std::move(cpdV7Input);
				if (!ConvertLitematicData_V7_To_V6(cpdV7Input, cpdV6Output))//从cpdV7Input转换到cpdV6Output
				{
					throw std::runtime_error("Unable to convert v7_data to v6_data!");
				}

				if (!NBT_Writer::WriteNBT(outputV6Stream, cpdV6Output, 512, NBT_Print{ NULL,NULL,NULL }))
				{
					throw std::runtime_error("Unable to write data into stream!\n");
				}
			}
	
			// 返回结果
			return outputV6Stream.ToJByteArray();
		}
		catch (const std::exception &e)
		{
			// 可以选择抛出 Java 异常
			jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
			if (exceptionClass != nullptr)
			{
				env->ThrowNew(exceptionClass, e.what());
				env->DeleteLocalRef(exceptionClass);
			}
			return nullptr;
		}
		catch (...)
		{
			return nullptr;
		}
	}
}
