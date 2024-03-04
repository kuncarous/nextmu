#ifndef __SHARED_MEMORYBUFFER_H__
#define __SHARED_MEMORYBUFFER_H__

#pragma once

#include <memory>

class EReadMemoryBuffer
{
public:
	EReadMemoryBuffer() :
		_MemoryBuffer(nullptr),
		_MemorySize(0),
		_MemoryOffset(0)
	{

	}

	void Release()
	{
		_MemoryBuffer.release();
		_MemorySize = 0;
		_MemoryOffset = 0;
	}

	void Reset()
	{
		_MemoryBuffer.reset();
		_MemorySize = 0;
		_MemoryOffset = 0;
	}

	void Reset(const mu_size size)
	{
		_MemoryBuffer.reset(new (std::nothrow) mu_uint8[size]);
		_MemorySize = size;
		_MemoryOffset = 0;
	}

	void Swap(std::unique_ptr<mu_uint8[]> &buffer, const mu_size size)
	{
		_MemoryBuffer.swap(buffer);
		_MemorySize = size;
		_MemoryOffset = 0;
	}

	mu_uint8 *GetBuffer()
	{
		return _MemoryBuffer.get();
	}

	const mu_size GetSize()
	{
		return _MemorySize;
	}

	const mu_size GetOffset()
	{
		return _MemoryOffset;
	}

	template<typename T>
	void Read(T &value)
	{
		if (_MemoryOffset + sizeof(T) > _MemorySize)
		{
			assert(!"Reading outside of buffer");
		}

		mu_memcpy(&value, _MemoryBuffer.get() + _MemoryOffset, sizeof(T));
		_MemoryOffset += sizeof(T);
	}

	template<typename T, const mu_uint32 N>
	void Read(std::array<T, N> &value)
	{
		if (_MemoryOffset + sizeof(T) * N > _MemorySize)
		{
			assert(!"Reading outside of buffer");
		}

		mu_memcpy(value.data(), _MemoryBuffer.get() + _MemoryOffset, sizeof(T) * N);
		_MemoryOffset += sizeof(T) * N;
	}

	NEXTMU_INLINE void Read(void *data, const mu_size size)
	{
		if (_MemoryOffset + size > _MemorySize)
		{
			assert(!"Reading outside of buffer");
		}

		mu_memcpy(data, _MemoryBuffer.get() + _MemoryOffset, size);
		_MemoryOffset += size;
	}

private:
	std::unique_ptr<mu_uint8[]> _MemoryBuffer;
	mu_size _MemorySize;
	mu_size _MemoryOffset;
};

#endif