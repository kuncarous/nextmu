#ifndef __SHARED_BINARYREADER_H__
#define __SHARED_BINARYREADER_H__

#pragma once

class NBinaryReader
{
public:
	NBinaryReader(mu_uint8 *buffer, const mu_uint32 bufferSize) :
		Current(0),
		Size(bufferSize),
		Buffer(buffer)
	{}

	void Replace(mu_uint8 *buffer, const mu_uint32 size, const mu_uint32 current = 0)
	{
		Current = current;
		Size = size;
		Buffer = buffer;
	}

	template<typename T>
	T Read()
	{
#if NEXTMU_COMPILE_DEBUG
		mu_assert(Current + sizeof(T) <= Size);
#endif
		const T value = *reinterpret_cast<const T *>(GetPointer());
		Current += sizeof(T);
		return value;
	}

	void ReadLine(void *dest, const mu_uint32 bytes)
	{
		mu_memcpy(dest, GetPointer(), bytes);
		Current += bytes;
	}

	void Skip(const mu_uint32 bytes)
	{
		Current += bytes;
	}

	void Reset()
	{
		Current = 0;
	}

	const mu_uint8 *GetBuffer()
	{
		return Buffer;
	}

	template<typename T = mu_uint8>
	T *GetPointer()
	{
		return reinterpret_cast<T *>(&Buffer[Current]);
	}

	const mu_uint32 GetCurrent()
	{
		return Current;
	}

	const mu_uint32 GetSize()
	{
		return Size;
	}

private:
	mu_uint32 Current;
	mu_uint32 Size;
	mu_uint8 *Buffer;
};

#endif