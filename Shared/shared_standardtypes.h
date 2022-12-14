#ifndef __SHARED_STANDARDTYPES_H__
#define __SHARED_STANDARDTYPES_H__

#include <iostream>
#include <atomic>
#include <cwchar>
#include <array>
#include <vector>

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free(ptr) free(ptr)
#endif

#ifndef NEXT_ALIGN
#if defined(_MSC_VER)
#define NEXT_BALIGN(x) __declspec(align(x))
#define NEXT_ALIGN(x, ...) NEXT_BALIGN(x) __VA_ARGS__
#else
#define NEXT_BALIGN(x) __attribute__((aligned(x)))
#define NEXT_ALIGN(x, ...) __VA_ARGS__ NEXT_BALIGN(x)
#endif
#endif

#if defined(__arm__)
#define ARM_PACKED __attribute__((packed))
#else
#define ARM_PACKED
#endif

typedef char				mu_char;
typedef wchar_t				mu_unicode;

typedef bool				mu_boolean;
typedef signed char			mu_int8;
typedef signed short		mu_int16;
typedef signed int			mu_int32;
typedef signed long long	mu_int64;
typedef signed long			mu_long;
typedef unsigned char		mu_uint8;
typedef unsigned short		mu_uint16;
typedef unsigned int		mu_uint32;
typedef unsigned long long	mu_uint64;
typedef unsigned long		mu_ulong;

typedef size_t				mu_count;
typedef size_t				mu_size;
#if NEXTMU_OPERATING_SYSTEM_BITS == NEXTMU_OS_32BITS
typedef mu_int32			mu_isize;
#else
typedef mu_int64			mu_isize;
#endif

typedef double				mu_double;
typedef float				mu_float;

typedef void* mu_ptr;

typedef std::string			mu_utf8string;
typedef std::wstring		mu_unicodestring;
typedef std::stringstream	mu_utf8stream;
typedef std::wstringstream	mu_unicodestream;

typedef std::wifstream		mu_unicodeifstream;
typedef std::wofstream		mu_unicodeofstream;
typedef std::wfstream		mu_unicodefstream;

typedef std::ifstream		mu_utf8ifstream;
typedef std::ofstream		mu_utf8ofstream;
typedef std::fstream		mu_utf8fstream;

typedef std::stringstream	mu_utf8stringstream;
typedef std::wstringstream	mu_unicodestringstream;

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
typedef std::atomic_bool		mu_atomic_bool;
typedef std::atomic<mu_int16>	mu_atomic_int16_t;
typedef std::atomic<mu_uint16>	mu_atomic_uint16_t;
typedef std::atomic<mu_int32>	mu_atomic_int32_t;
typedef std::atomic<mu_uint32>	mu_atomic_uint32_t;
typedef std::atomic<mu_int64>	mu_atomic_int64_t;
typedef std::atomic<mu_uint64>	mu_atomic_uint64_t;
#else
typedef std::atomic_bool		mu_atomic_bool;
typedef std::atomic_int16_t		mu_atomic_int16_t;
typedef std::atomic_uint16_t	mu_atomic_uint16_t;
typedef std::atomic_int32_t		mu_atomic_int32_t;
typedef std::atomic_uint32_t	mu_atomic_uint32_t;
typedef std::atomic_int64_t		mu_atomic_int64_t;
typedef std::atomic_uint64_t	mu_atomic_uint64_t;
#endif

constexpr mu_int8 NInvalidInt8 = -1;
constexpr mu_int16 NInvalidInt16 = -1;
constexpr mu_int32 NInvalidInt32 = -1;
constexpr mu_int64 NInvalidInt64 = -1;
constexpr mu_float NInvalidFloat = -1.0f;
constexpr mu_double NInvalidDouble = -1.0;

constexpr mu_uint8 NInvalidUInt8 = 0xFF;
constexpr mu_uint16 NInvalidUInt16 = 0xFFFF;
constexpr mu_uint32 NInvalidUInt32 = 0xFFFFFFFF;
constexpr mu_uint64 NInvalidUInt64 = 0xFFFFFFFFFFFFFFFF;
constexpr mu_size NInvalidSize = static_cast<mu_size>(-1);

#ifndef _ET
#define _ET(x)      x
#endif

#ifndef _GETTEXT
#define __GETTEXT(x) #x
#define _GETTEXT(value) __GETTEXT(value)
#endif

#define mu_countof(array) (sizeof(array)/sizeof(array[0]))

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
#define _stricmp strcasecmp
#define strncpy_s strncpy

NEXTMU_INLINE int sprintf_s(char* s, mu_size n, const char* fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsnprintf(s, n, fmt, ap);
	if (ret > 0 && static_cast<mu_size>(ret) < n) s[ret] = '\0';
	va_end(ap);
	return ret;
}
NEXTMU_INLINE void strcpy_s(char* dest, const mu_size destLength, const char* src)
{
	strcpy(dest, src);
}
NEXTMU_INLINE void strncpy_s(char* dest, const mu_size destLength, const char* src, const mu_size copyLength)
{
	strncpy(dest, src, copyLength);
}
NEXTMU_INLINE void strcat_s(char* dest, const mu_size destLength, const char* src)
{
	strcat(dest, src);
}

#define _wcsicmp wcscasecmp
#define _wcsnicmp wcsncasecmp
#define swscanf_s swscanf

NEXTMU_INLINE int swprintf_s(wchar_t* s, mu_size n, const wchar_t* fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vswprintf(s, n, fmt, ap);
	if (ret > 0 && static_cast<mu_size>(ret) < n) s[ret] = L'\0';
	va_end(ap);
	return ret;
}
NEXTMU_INLINE void wcscpy_s(wchar_t* dest, const mu_size destLength, const wchar_t* src)
{
	wcscpy(dest, src);
}
NEXTMU_INLINE void wcsncpy_s(wchar_t* dest, const mu_size destLength, const wchar_t* src, const mu_size copyLength)
{
	wcsncpy(dest, src, copyLength);
}
NEXTMU_INLINE void wcscat_s(wchar_t* dest, const mu_size destLength, const wchar_t* src)
{
	wcscat(dest, src);
}
#endif

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
#include <stdio.h>
#define vswprintf_s vswprintf
#endif

// Unicode
#define mu_unicodestrcmp		wcscmp
#define mu_unicodestrlen		wcslen
#define mu_unicodestrcpy_s		wcscpy_s
#define mu_unicodestrncpy_s		wcsncpy_s
#define mu_unicodesprintf		swprintf
#define mu_unicodesprintf_s		swprintf_s
#define mu_unicodevsprintf_s	vswprintf_s
#define mu_unicodetolower		towlower
#define mu_unicodetoupper		towupper
#define mu_unicodestrcmp		wcscmp
#define mu_unicodestrncmp		wcsncmp
#define mu_unicodestrtok		wcstok_s
#define mu_unicodestrcat_s		wcscat_s

#define mu_unicodesplitpath		_wsplitpath_s
#define mu_unicodestrcpy		wcscpy_s
#define mu_unicodestrncpy		wcsncpy_s
#define mu_unicodestrcpy_ns		wcscpy
#define mu_unicodestrncpy_ns	wcsncpy
#define mu_unicodestrcat		wcscat_s
#define mu_unicodestrcat_ns		wcscat
#define mu_unicodestricmp		_wcsicmp
#define mu_unicodestrnicmp		_wcsnicmp
#define mu_unicodestrcmp		wcscmp
#define mu_unicodeprintf		wprintf
#define mu_unicodestrtol		wcstol
#define mu_unicodestrtoul		wcstoul
#define mu_unicodestrstr		wcsstr
#define mu_unicodesscanf		swscanf_s

#define mu_unicodegetc			getc
#define mu_unicodeungetc		ungetc
#define mu_unicodeisspace		iswspace
#define mu_unicodeisalpha		iswalpha
#define mu_unicodeisdigit		iswdigit
#define mu_unicodeisalnum		iswalnum
#define mu_unicodeisxdigit		iswxdigit
#define mu_unicodeatof			_wtof
#define mu_unicodeitoa			_itow_s
#define mu_unicodeatoi			_wtoi
#define mu_unicodestoi			std::stoi

// UTF-8
#define mu_utf8strcmp			strcmp
#define mu_utf8strlen			strlen
#define mu_utf8strcpy_s			strcpy_s
#define mu_utf8strncpy_s		strncpy_s
#define mu_utf8sprintf			sprintf
#define mu_utf8sprintf_s		sprintf_s
#define mu_utf8vsprintf_s		vsprintf_s
#define mu_utf8tolower			::tolower
#define mu_utf8toupper			::toupper
#define mu_utf8strcmp			strcmp
#define mu_utf8strncmp			strncmp
#define mu_utf8strtok			strtok_s
#define mu_utf8strcat_s			strcat_s

#define mu_utf8splitpath		_splitpath_s
#define mu_utf8strcpy			strcpy_s
#define mu_utf8strncpy			strncpy_s
#define mu_utf8strcpy_ns		strcpy
#define mu_utf8strncpy_ns		strncpy
#define mu_utf8strcat			strcat_s
#define mu_utf8strcat_ns		strcat
#define mu_utf8stricmp			_stricmp
#define mu_utf8strnicmp			_strnicmp
#define mu_utf8strcmp			strcmp
#define mu_utf8printf			printf
#define mu_utf8strtol			strtol
#define mu_utf8strtoul			strtoul
#define mu_utf8strstr			strstr
#define mu_utf8sscanf			sscanf_s

#define mu_utf8getc				getc
#define mu_utf8ungetc			ungetc
#define mu_utf8isspace			isspace
#define mu_utf8isalpha			isalpha
#define mu_utf8isdigit			isdigit
#define mu_utf8isalnum			isalnum
#define mu_utf8isxdigit			isxdigit
#define mu_utf8atof				atof
#define mu_utf8itoa				_itoa_s
#define mu_utf8atoi				atoi
#define mu_utf8stoi				stoi

#define mu_fclose				fclose
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
#define mu_fseek				fseek
#define mu_ftellg				ftell
#else
#define mu_fseek				_fseeki64
#define mu_ftellg				_ftelli64
#endif
#define mu_fread				fread
#define mu_fwrite				fwrite

#define mu_memcmp				memcmp
#define mu_memset				memset
#define mu_zeromem(D, L)		mu_memset(D, 0, L)
#define mu_memcpy				memcpy
#define mu_memmove				memmove
#define mu_malloc				malloc
#define mu_realloc				realloc
#define mu_free					free
#define mu_sleep(ms)			boost::this_thread::sleep_for(boost::chrono::milliseconds(ms))

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
typedef int errno_t;
#else
#define mu_messagebox_utf8		::MessageBoxA
#define mu_messagebox_unicode	::MessageBoxW
#define mu_messagebox			::MessageBox
#define mu_getdc				::GetDC
#endif

namespace MU
{
#define MAKE_RGBA(r,g,b,a) ((mu_uint32)((((r)&0xff)<<24)|(((g)&0xff)<<16)|(((b)&0xff)<<8)|((a)&0xff)))
#define MAKE_ARGB(a,r,g,b) ((mu_uint32)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

	inline constexpr mu_uint32 GetRGB(const mu_uint32 ARGB)
	{
		return ARGB & 0x00FFFFFF;
	}
	inline constexpr mu_uint8 GetAlpha(const mu_uint32 ARGB)
	{
		return static_cast<mu_uint8>((ARGB) >> 24);
	}
	inline constexpr mu_uint8 GetBlue(const mu_uint32 ARGB)
	{
		return static_cast<mu_uint8>(((ARGB) >> 16) & 0xff);
	}
	inline constexpr mu_uint8 GetGreen(const mu_uint32 ARGB)
	{
		return static_cast<mu_uint8>(((ARGB) >> 8) & 0xff);
	}
	inline constexpr mu_uint8 GetRed(const mu_uint32 ARGB)
	{
		return static_cast<mu_uint8>((ARGB) & 0xff);
	}
	inline constexpr mu_uint32 MakeRGBA(const mu_uint8 R, const mu_uint8 G, const mu_uint8 B, const mu_uint8 A)
	{
		return MAKE_RGBA(R, G, B, A);
	}
	inline constexpr mu_uint32 MakeARGB(const mu_uint8 R, const mu_uint8 G, const mu_uint8 B, const mu_uint8 A)
	{
		return MAKE_ARGB(A, R, G, B);
	}
	inline constexpr mu_uint32 MakeARGB(const mu_uint32 Color, const mu_uint8 A)
	{
		return (GetRGB(Color) | (static_cast<mu_uint32>(A) << 24));
	}

	NEXTMU_INLINE void PathFix(mu_utf8string& path, mu_boolean isDir)
	{
#ifdef _WIN32
		std::replace(path.begin(), path.end(), '/', '\\');
#endif

		if (isDir == true && path.back() != '\\')
		{
			path.push_back('\\');
		}
	}

	NEXTMU_INLINE void PathFix(mu_unicodestring& path, mu_boolean isDir)
	{
#ifdef _WIN32
		std::replace(path.begin(), path.end(), _ET('/'), _ET('\\'));
#endif

		if (isDir == true && path.back() != _ET('\\'))
		{
			path.push_back(_ET('\\'));
		}
	}
};

struct utf8_icmp
{
	mu_boolean operator () (const mu_char* a, const mu_char* b) const
	{
		return mu_utf8stricmp(a, b) < 0;
	}
	mu_boolean operator () (const mu_utf8string& a, const mu_utf8string& b) const
	{
		return mu_utf8stricmp(a.c_str(), b.c_str()) < 0;
	}
};

struct unicode_icmp
{
	mu_boolean operator () (const mu_unicode* a, const mu_unicode* b) const
	{
		return mu_unicodestricmp(a, b) < 0;
	}
	mu_boolean operator () (const mu_unicodestring& a, const mu_unicodestring& b) const
	{
		return mu_unicodestricmp(a.c_str(), b.c_str()) < 0;
	}
};

struct textci_less
{
	struct nocase_compare
	{
		mu_boolean operator() (const mu_char& c1, const mu_char& c2) const
		{
			return mu_utf8tolower(c1) < mu_utf8tolower(c2);
		}
	};

	mu_boolean operator() (const mu_utf8string& s1, const mu_utf8string& s2) const
	{
		return std::lexicographical_compare
		(s1.begin(), s1.end(),
			s2.begin(), s2.end(),
			nocase_compare());
	}
};

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
#include <windows.h>
#endif

/*
	Cross Platform to/from UTF-8 conversion functions!
	This will be used for decrease network usage and allow cross-platform compatibility
	Return buffer count used(exactly one buffer count, not characters count)
	* InputCount is characters count(not buffer size)
	* OutputSize is the max output size in bytes

	WARNING!!!
	Ensure always have an extra character slot for null-termination
*/
NEXTMU_INLINE mu_boolean ConvertToUTF8(const mu_unicode* input, mu_uint32 inputCount, mu_char* output, mu_uint32 outputSize, mu_uint32* outputCount)
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

NEXTMU_INLINE mu_boolean ConvertFromUTF8(const mu_char* input, mu_uint32 inputCount, mu_unicode* output, mu_uint32 outputSize, mu_uint32* outputCount)
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

NEXTMU_INLINE mu_utf8string ConvertToUTF8String(const mu_unicodestring str)
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

NEXTMU_INLINE mu_unicodestring ConvertToUnicodeString(const mu_utf8string str)
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

NEXTMU_INLINE mu_uint16 MakeWord(mu_uint8 Low, mu_uint8 High)
{
	return static_cast<mu_uint16>(static_cast<mu_uint32>(Low) | (static_cast<mu_uint32>(High) << 8));
}

NEXTMU_INLINE mu_uint8 shex2bin(const mu_char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	else if (c >= 'A' && c <= 'F')
		return (10 + (c - 'A'));
	else if (c >= 'a' && c <= 'f')
		return (10 + (c - 'a'));
	return 0x00;
}

NEXTMU_INLINE void hex2bin(const mu_utf8string input, mu_uint8* output, const mu_uint32 size)
{
	if (input.size() % 2 == 1 || input.size() != size * 2) return;
	for (mu_uint32 n = 0, c = 0; n < size; ++n, c += 2)
	{
		output[n] = shex2bin(input[c]) << 4 | shex2bin(input[c + 1]);
	}
}

#ifndef NDEBUG
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS || \
	NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
#define mu_debugbreak() raise(SIGTRAP)
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
#define mu_debugbreak() __debugbreak()
#else
#define mu_debugbreak() (0)
#endif
#else
#define mu_debugbreak() (0)
#endif

#ifndef NDEBUG
#define EASSERT_FUNCTION __func__
#define EASSERT_FILE __FILE__
#define EASSERT_LINE __LINE__
#else
#define EASSERT_FUNCTION ""
#define EASSERT_FILE ""
#define EASSERT_LINE 0
#endif

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
#include <android/log.h>

template<typename... T>
void AndroidAssert(const mu_char* asserttype, fmt::format_string<T...> text, T&&... args)
{
	const mu_utf8string srcmessage = fmt::format(text, std::forward<T>(args)...);
	const mu_utf8string message = fmt::format("[NextMU][{0}] {1}\n", asserttype, srcmessage);
	__android_log_print(ANDROID_LOG_ERROR, "NextMU", "%s", message.c_str());
}

template<typename... T>
void mu_error(fmt::format_string<T...> x, T&&... args)
{
	AndroidAssert("Error", x, std::forward<T>(args)...);
}
template<typename... T>
void mu_info(fmt::format_string<T...> x, T&&... args)
{
	AndroidAssert("Info", x, std::forward<T>(args)...);
}

template<typename... T>
void mu_debug_error(fmt::format_string<T...> x, T&&... args)
{
#ifndef NDEBUG
	AndroidAssert("Error", x, std::forward<T>(args)...);
#endif
}
template<typename... T>
void mu_debug_info(fmt::format_string<T...> x, T&&... args)
{
#ifndef NDEBUG
	AndroidAssert("Info", x, std::forward<T>(args)...);
#endif
}
template<typename... T>
void mu_trace_info(fmt::format_string<T...> x, T&&... args)
{
#if !defined(NDEBUG) && defined(NEXTMU_ENABLE_TRACE_INFO)
	AndroidAssert("Info", x, std::forward<T>(args)...);
#endif
}

#ifndef NDEBUG
#define mu_assert(x) if (!(x)) { AndroidAssert("Assert", #x); }
#else
#define mu_assert(x) (void)(x)
#endif
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
#ifndef NDEBUG
template<typename... T>
void WindowsAssert(const mu_char* asserttype, fmt::format_string<T...> text, T&&... args)
{
	const mu_utf8string srcmessage = fmt::format(text, std::forward<T>(args)...);
	const mu_utf8string message = fmt::format("[NextMU][{0}] {1}\n", asserttype, srcmessage);
	OutputDebugStringA(message.c_str());
	OutputDebugStringA("\n");
}
#else
template<typename... T>
void WindowsAssert(const mu_char* asserttype, fmt::format_string<T...> text, T&&... args)
{
#if 1
	const mu_utf8string srcmessage = fmt::format(text, std::forward<T>(args)...);
	const mu_utf8string message = fmt::format("[NextMU][{0}] {1}\n", asserttype, srcmessage);
	OutputDebugStringA(message.c_str());
	OutputDebugStringA("\n");
#else
	const mu_utf8string srcmessage = fmt::format(text, std::forward<T>(args)...);
	mu_messagebox_utf8(nullptr, fmt::format("[NextMU][{0}] {1}\n", asserttype, srcmessage).c_str(), "Error", MB_OK);
#endif
}
#endif

template<typename... T>
void mu_error(fmt::format_string<T...> x, T&&... args)
{
	WindowsAssert("Error", x, std::forward<T>(args)...);
}
template<typename... T>
void mu_info(fmt::format_string<T...> x, T&&... args)
{
	WindowsAssert("Info", x, std::forward<T>(args)...);
}

template<typename... T>
void mu_debug_error(fmt::format_string<T...> x, T&&... args)
{
#ifndef NDEBUG
	WindowsAssert("Error", x, std::forward<T>(args)...);
#endif
}
template<typename... T>
void mu_debug_info(fmt::format_string<T...> x, T&&... args)
{
#ifndef NDEBUG
	WindowsAssert("Info", x, std::forward<T>(args)...);
#endif
}
template<typename... T>
void mu_trace_info(fmt::format_string<T...> x, T&&... args)
{
#if !defined(NDEBUG) && defined(NEXTMU_ENABLE_TRACE_INFO)
	WindowsAssert("Info", x, std::forward<T>(args)...);
#endif
}

#ifndef NDEBUG
#define mu_assert(x) if (!(x)) { WindowsAssert("Assert", #x); }
#else
#define mu_assert(x) (void)(x)
#endif
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
void iOSLogToConsole(const mu_char* message);

template<typename... T>
void iOSAssert(const mu_char* asserttype, fmt::format_string<T...> text, T&&... args)
{
	const mu_utf8string srcmessage = fmt::format(text, std::forward<T>(args)...);
	const mu_utf8string message = fmt::format("[NextMU][{0}] {1}\n", asserttype, srcmessage);

	iOSLogToConsole(message.c_str());
}

template<typename... T>
void mu_error(fmt::format_string<T...> x, T&&... args)
{
	iOSAssert("Error", x, std::forward<T>(args)...);
}
template<typename... T>
void mu_info(fmt::format_string<T...> x, T&&... args)
{
	iOSAssert("Info", x, std::forward<T>(args)...);
}

template<typename... T>
void mu_debug_error(fmt::format_string<T...> x, T&&... args)
{
#ifndef NDEBUG
	iOSAssert("Error", x, std::forward<T>(args)...);
#endif
}
template<typename... T>
void mu_debug_info(fmt::format_string<T...> x, T&&... args)
{
#ifndef NDEBUG
	iOSAssert("Info", x, std::forward<T>(args)...);
#endif
}
template<typename... T>
void mu_trace_info(fmt::format_string<T...> x, T&&... args)
{
#if !defined(NDEBUG) && defined(NEXTMU_ENABLE_TRACE_INFO)
	iOSAssert("Info", x, std::forward<T>(args)...);
#endif
}

#ifndef NDEBUG
#define mu_assert(x) if (!(x)) { iOSAssert("Assert", #x); }
#else
#define mu_assert(x) (void)(x)
#endif
#else
#include <assert.h>
#define mu_error(x) mu_assert(!x)
#define mu_info
#ifndef NDEBUG
#define mu_assert assert
#define mu_debug_error(x, ...) mu_assert(!x)
#define mu_debug_info(x, ...) mu_assert(!x)
#if defined(NEXTMU_ENABLE_TRACE_INFO)
#define mu_trace_info(x, ...) mu_assert(!x)
#else
#define mu_trace_info
#endif
#else
#define mu_assert
#define mu_debug_error(x, ...)
#define mu_debug_info(x, ...)
#define mu_trace_info
#endif
#endif

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID // Prevent Warnings
#include <linux/swab.h>
static inline void __swab32s_prevent(__u32* p) {
	__swab32s(p);
}
static inline void __swab64s_prevent(__u64* p) {
	__swab64s(p);
}
#endif

template <typename OutputIt, typename S, typename... Args>
void mu_format_to_n(const OutputIt out, std::size_t n, const S& format_str, const Args &... args)
{
	auto t = fmt::format_to_n(out, n, format_str, args...);
	out[t.size == n ? n - 1 : t.size] = 0;
}

constexpr mu_uint32 ComputeBitsNeeded(const mu_uint32 ValuesCount)
{
	if (ValuesCount == 0) return 0u;
	mu_uint32 bitsCount = 1;
	mu_uint32 n = 1;
	while (n < ValuesCount - 1)
	{
		n <<= 1;
		n |= 1;
		++bitsCount;
	}
	return bitsCount;
}

template<const mu_boolean AddTrailing = false>
void NormalizePath(mu_utf8string &path)
{
	std::replace(path.begin(), path.end(), '\\', '/');
	if constexpr (AddTrailing)
	{
		if (!path.ends_with('/')) path += '/';
	}
}

#endif