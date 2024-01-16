#include "shared_precompiled.h"

/*
	Cross Platform to/from UTF-8 conversion functions!
	This will be used for decrease network usage and allow cross-platform compatibility
	Return buffer count used(exactly one buffer count, not characters count)
	* InputCount is characters count(not buffer size)
	* OutputSize is the max output size in bytes

	WARNING!!!
	Ensure always have an extra character slot for null-termination
*/
mu_boolean ConvertToUTF8(const mu_unicode* input, mu_uint32 inputCount, mu_char* output, mu_uint32 outputSize, mu_uint32* outputCount)
{
	mu_uint32 tmpCount;

	if (outputCount == nullptr)
	{
		outputCount = &tmpCount;
	}

	if (inputCount == 0)
	{
		return false;
	}

	if (input[inputCount - 1] == _ET('\0'))
	{
		inputCount--;
	}

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
	* outputCount = static_cast<mu_uint32>(wcstombs(output, input, static_cast<size_t>(inputCount)));
#else
	* outputCount = WideCharToMultiByte(CP_UTF8, 0, input, inputCount, output, outputSize, nullptr, nullptr);
#endif

	if (output != nullptr &&
		*outputCount < outputSize)
	{
		output[*outputCount] = '\0';
	}

	return *outputCount > 0;
}

mu_boolean ConvertFromUTF8(const mu_char* input, mu_uint32 inputCount, mu_unicode* output, mu_uint32 outputSize, mu_uint32* outputCount)
{
	mu_uint32 tmpCount;

	if (outputCount == nullptr)
	{
		outputCount = &tmpCount;
	}

	if (inputCount == 0)
	{
		return false;
	}

	if (input[inputCount - 1] == '\0')
	{
		inputCount--;
	}

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
	* outputCount = static_cast<mu_uint32>(mbstowcs(output, input, static_cast<size_t>(inputCount)));
#else
	* outputCount = MultiByteToWideChar(CP_UTF8, 0, input, inputCount, output, outputSize);
#endif

	if (output != nullptr &&
		*outputCount < outputSize)
	{
		output[*outputCount] = '\0';
	}

	return *outputCount > 0;
}

mu_utf8string ConvertToUTF8String(const mu_unicodestring str)
{
	mu_uint32 neededSize = 0;
	ConvertToUTF8(str.c_str(), static_cast<mu_uint32>(str.size()), nullptr, 0, &neededSize);
	if (neededSize == 0)
		return "";

	std::unique_ptr<mu_char[]> c(new_nothrow mu_char[neededSize + 1]);
	mu_zeromem(c.get(), sizeof(mu_char) * (neededSize + 1));
	ConvertToUTF8(str.c_str(), static_cast<mu_uint32>(str.size()), c.get(), neededSize, &neededSize);
	if (neededSize == 0)
		return "";

	return mu_utf8string(c.get());
}

mu_unicodestring ConvertToUnicodeString(const mu_utf8string str)
{
	mu_uint32 neededSize = 0;
	ConvertFromUTF8(str.c_str(), static_cast<mu_uint32>(str.size()), nullptr, 0, &neededSize);
	if (neededSize == 0)
		return L"";

	std::unique_ptr<mu_unicode[]> c(new_nothrow mu_unicode[neededSize + 1]);
	mu_zeromem(c.get(), sizeof(mu_unicode) * (neededSize + 1));
	ConvertFromUTF8(str.c_str(), static_cast<mu_uint32>(str.size()), c.get(), neededSize, &neededSize);
	if (neededSize == 0)
		return L"";

	return mu_unicodestring(c.get());
}