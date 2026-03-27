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
	jsize size;
	jsize position;
	bool isReleased;

public:
	using StreamType = jbyteArray;
	using ValueType = jbyte;

	/// @brief 构造函数，从 jbyteArray 创建输入流
	/// @param env JNI 环境指针
	/// @param input Java 字节数组
	JNIInputStream(JNIEnv *env, jbyteArray input)
		: env(env), data(input), buffer(nullptr), size(0), position(0), isReleased(false)
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
		Release();
	}

	// 禁止拷贝和移动
	JNIInputStream(const JNIInputStream &) = delete;
	JNIInputStream(JNIInputStream &&) = delete;
	JNIInputStream &operator=(const JNIInputStream &) = delete;
	JNIInputStream &operator=(JNIInputStream &&) = delete;

	/// @brief 下标访问运算符
	const ValueType &operator[](size_t index) const noexcept
	{
		return buffer[index];
	}

	/// @brief 获取下一个字节并推进读取位置
	const ValueType &GetNext() noexcept
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
	const ValueType *CurData() const noexcept
	{
		return &buffer[position];
	}

	/// @brief 向后推进读取
	jsize AddIndex(size_t szSize) noexcept
	{
		position += szSize;
		return position;
	}

	/// @brief 向前撤销读取
	jsize SubIndex(size_t szSize) noexcept
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
	jsize Size() const noexcept
	{
		return size;
	}

	/// @brief 检查是否还有足够的数据
	bool HasAvailData(jsize jSize) const noexcept
	{
		return (size - position) >= jSize;
	}

	/// @brief 重置流读取位置
	void Reset() noexcept
	{
		position = 0;
	}

	/// @brief 获取底层数据的起始指针
	const ValueType *BaseData() const noexcept
	{
		return buffer;
	}

	/// @brief 获取当前读取位置（只读）
	jsize Index() const noexcept
	{
		return position;
	}

	/// @brief 获取当前读取位置（可写）
	jsize &Index() noexcept
	{
		return position;
	}

	/// @brief 释放 JNI 资源（手动调用）
	void Release()
	{
		if (!isReleased && buffer != nullptr)
		{
			env->ReleaseByteArrayElements(data, buffer, JNI_ABORT);
			isReleased = true;
			buffer = nullptr;
		}
	}
};

/// @brief JNI 输出流适配器，使用 JVM 内存分配器
class JNIOutputStream
{
private:
	JNIEnv *env;
	jbyteArray currentBuffer;      // 当前使用的缓冲区
	jbyte *bufferPtr;              // 缓冲区的直接指针
	jsize bufferCapacity;         // 缓冲区总容量
	jsize position;               // 当前写入位置

	// 动态扩容策略：每次扩容到原来的 1.5 倍
	static constexpr size_t INITIAL_CAPACITY = 1024;
	static constexpr size_t MAX_CAPACITY = INT32_MAX;//jsize = jint = int32_t

	/// @brief 确保有足够的空间写入 szSize 字节
	void EnsureCapacity(size_t szSize)
	{
		if (position < 0)
		{
			throw std::runtime_error("The value of position is less than zero");
		}

		size_t neededSize = (size_t)position + szSize;
		if (neededSize <= bufferCapacity)
		{
			return;//空间足够
		}
		else if (neededSize > MAX_CAPACITY)//溢出
		{
			throw std::runtime_error("Output buffer exceeds maximum capacity");
		}
		//else//正常

		// 计算新容量：至少1.5倍，但要满足 neededSize
		size_t newCapacity = bufferCapacity;
		do
		{
			newCapacity = (newCapacity + (newCapacity / 2));//*1.5扩容
			if (newCapacity > MAX_CAPACITY)
			{
				newCapacity = MAX_CAPACITY;
				break;
			}
		} while (newCapacity < neededSize);

		// 重新分配 JVM 数组
		Reallocate((jsize)newCapacity);
	}

	/// @brief 重新分配缓冲区，保留已有数据
	void Reallocate(jsize newCapacity)
	{
		// 创建新数组
		jbyteArray newBuffer = env->NewByteArray(newCapacity);
		if (newBuffer == nullptr)
		{
			throw std::runtime_error("Failed to allocate JNI byte array");
		}

		// 获取新数组的指针
		jbyte *newPtr = env->GetByteArrayElements(newBuffer, nullptr);
		if (newPtr == nullptr)
		{
			env->DeleteLocalRef(newBuffer);
			throw std::runtime_error("Failed to get new buffer elements");
		}

		// 复制现有数据
		if (bufferPtr != nullptr && position > 0)
		{
			memcpy(newPtr, bufferPtr, position);
		}

		// 释放旧缓冲区
		if (currentBuffer != nullptr)
		{
			if (bufferPtr != nullptr)
			{
				env->ReleaseByteArrayElements(currentBuffer, bufferPtr, 0);
			}
			env->DeleteLocalRef(currentBuffer);
		}

		// 更新为新缓冲区
		currentBuffer = newBuffer;
		bufferPtr = newPtr;
		bufferCapacity = newCapacity;
	}

public:
	using StreamType = jbyteArray;
	using ValueType = jbyte;

	/// @brief 构造函数
	JNIOutputStream(JNIEnv *env, jsize initSize = INITIAL_CAPACITY)
		: env(env), currentBuffer(nullptr), bufferPtr(nullptr),
		bufferCapacity(0), position(0)
	{
		// 初始分配
		Reallocate(initSize);
	}

	/// @brief 析构函数，释放 JVM 资源
	~JNIOutputStream(void)
	{
		if (currentBuffer != nullptr)
		{
			if (bufferPtr != nullptr)
			{
				// 不写回数据，直接放弃
				env->ReleaseByteArrayElements(currentBuffer, bufferPtr, JNI_ABORT);
			}
			env->DeleteLocalRef(currentBuffer);
		}
	}

	// 禁止拷贝
	JNIOutputStream(const JNIOutputStream &) = delete;
	JNIOutputStream(JNIOutputStream &&_Move) noexcept:
		env(_Move.env),
		currentBuffer(_Move.currentBuffer),
		bufferPtr(_Move.bufferPtr),
		bufferCapacity(_Move.bufferCapacity),
		position(_Move.position)
	{
		//_Move.env;//不清理
		_Move.currentBuffer = nullptr;
		_Move.bufferPtr = nullptr;
		_Move.bufferCapacity = 0;
		_Move.position = 0;
	}
	JNIOutputStream &operator=(const JNIOutputStream &) = delete;
	JNIOutputStream &operator=(JNIOutputStream &&) = delete;

	/// @brief 下标访问运算符（只读）
	const ValueType &operator[](size_t index) const noexcept
	{
		return bufferPtr[index];
	}

	/// @brief 向流中写入单个值
	template<typename V>
	requires(std::is_constructible_v<ValueType, V &&>)
	void PutOnce(V &&c)
	{
		EnsureCapacity(1);
		bufferPtr[position++] = ValueType{ std::forward<V>(c) };
	}

	/// @brief 向流中写入一段数据
	void PutRange(const ValueType *pData, size_t szSize)
	{
		if (szSize == 0)
		{
			return;
		}

		EnsureCapacity(szSize);
		memcpy(&bufferPtr[position], pData, szSize);
		position += (jsize)szSize;
	}

	/// @brief 预分配额外容量
	void AddReserve(size_t szAddSize)
	{
		EnsureCapacity(szAddSize);
	}

	/// @brief 删除最后一个写入的字节
	void UnPut() noexcept
	{
		if (position > 0)
		{
			--position;
		}
	}

	/// @brief 获取当前字节流大小
	size_t Size() const noexcept
	{
		return (size_t)position;
	}

	/// @brief 重置流，清空所有数据
	void Reset() noexcept
	{
		position = 0;
	}

	/// @brief 获取底层数据的指针
	const ValueType *Data() const noexcept
	{
		return bufferPtr;
	}

	/// @brief 创建 jbyteArray 并转移数据所有权
	/// @return 新创建的 jbyteArray，包含当前所有数据
	/// @note 调用后当前输出流会被重置，不应继续使用
	jbyteArray ToJByteArray()
	{
		if (position <= 0)
		{
			// 返回空数组
			return env->NewByteArray(0);
		}

		// 创建精确大小的结果数组
		jbyteArray result = env->NewByteArray(position);
		if (result == nullptr)
		{
			return nullptr;
		}

		// 复制数据到结果数组
		env->SetByteArrayRegion(result, 0, static_cast<jsize>(position),
			reinterpret_cast<const jbyte *>(bufferPtr));

		return result;
	}

	/// @brief 获取当前缓冲区指针（直接写入）
	/// @return 当前写入位置的指针
	ValueType *GetCurrentPointer() noexcept
	{
		return &bufferPtr[position];
	}

	/// @brief 增加写入位置（与 GetCurrentPointer 配合使用）
	void AdvancePosition(size_t szSize) noexcept
	{
		position += szSize;
	}

	/// @brief 获取当前写入位置
	size_t GetPosition() const noexcept
	{
		return position;
	}

	/// @brief 获取缓冲区剩余容量
	size_t RemainingCapacity() const noexcept
	{
		return bufferCapacity - position;
	}

	/// @brief 确认所有数据已提交（实际上总是已提交）
	void Flush() noexcept
	{
		// JVM 直接内存操作，无需 flush
	}
};

template<typename ISTREAM, typename OSTREAM>
bool ConvertLitematicFile_V7_To_V6(ISTREAM vDataV7Stream, OSTREAM vDataV6Stream)
{
	NBT_Type::Compound cpdV7Input{};
	NBT_Type::Compound cpdV6Output{};

	if (!NBT_Reader::ReadNBT(vDataV7Stream, cpdV7Input))
	{
		//printf("Unable to parse data from stream!\n");
		return false;
	}

	//从cpdV7Input转换到cpdV6Output
	if (!ConvertLitematicData_V7_To_V6(cpdV7Input, cpdV6Output))
	{
		//printf("Unable to convert v7_data to v6_data!\n");
		return false;
	}

	if (!NBT_Writer::WriteNBT(vDataV6Stream, cpdV6Output))
	{
		//printf("Unable to write data into stream!\n");
		return false;
	}

	return true;
}



JNIEXPORT void JNICALL Java_getVersion(JNIEnv *env, jobject obj)
{
	jint version = env->GetVersion();
}





