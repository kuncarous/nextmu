#ifndef __SHARED_OPERATING_SYSTEM_H__
#define __SHARED_OPERATING_SYSTEM_H__

#pragma once

#define NEXTMU_ARCH_X86			(0)
#define NEXTMU_ARCH_X86_64		(1)
#define NEXTMU_ARCH_ARM32		(2)
#define NEXTMU_ARCH_ARM64		(3)

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_M_IX86)
#define NEXTMU_ARCH				NEXTMU_ARCH_X86
#elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define NEXTMU_ARCH				NEXTMU_ARCH_X86_64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define NEXTMU_ARCH				NEXTMU_ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
#define NEXTMU_ARCH				NEXTMU_ARCH_ARM32
#else
#error "Unknown target architecture"
#endif

#if defined(NDEBUG)
#define NEXTMU_COMPILE_DEBUG		(0)
#else
#define NEXTMU_COMPILE_DEBUG		(1)
#endif

#define NEXTMU_OSTYPE_DESKTOP	(0)
#define NEXTMU_OSTYPE_MOBILE		(1)

#define NEXTMU_OS_WINDOWS		(0)
#define NEXTMU_OS_LINUX			(1)
#define NEXTMU_OS_MACOS			(2)
#define NEXTMU_OS_WINDOWS_MOBILE	(3)
#define NEXTMU_OS_ANDROID		(4)
#define NEXTMU_OS_IOS			(5)

#define NEXTMU_OS_32BITS			(0)
#define NEXTMU_OS_64BITS			(1)

#if defined(_MSC_VER)
#define NEXTMU_OPERATING_SYSTEM		NEXTMU_OS_WINDOWS
#define NEXTMU_OPERATING_SYSTEM_TYPE NEXTMU_OSTYPE_DESKTOP
#elif defined(__ANDROID__)
#define NEXTMU_OPERATING_SYSTEM		NEXTMU_OS_ANDROID
#define NEXTMU_OPERATING_SYSTEM_TYPE NEXTMU_OSTYPE_MOBILE
#elif defined(__linux__)
#define NEXTMU_OPERATING_SYSTEM		NEXTMU_OS_LINUX
#define NEXTMU_OPERATING_SYSTEM_TYPE NEXTMU_OSTYPE_DESKTOP
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
#define NEXTMU_OPERATING_SYSTEM		NEXTMU_OS_IOS
#define NEXTMU_OPERATING_SYSTEM_TYPE NEXTMU_OSTYPE_MOBILE
#elif TARGET_OS_MAC
#define NEXTMU_OPERATING_SYSTEM		NEXTMU_OS_MACOS
#define NEXTMU_OPERATING_SYSTEM_TYPE NEXTMU_OSTYPE_DESKTOP
#endif
#endif

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define NEXTMU_OPERATING_SYSTEM_BITS	NEXTMU_OS_64BITS
#else
#define NEXTMU_OPERATING_SYSTEM_BITS	NEXTMU_OS_32BITS
#endif

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
#define NEXTMU_INLINE __forceinline
#else
#define NEXTMU_INLINE inline
#endif

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
#define new_nothrow new
#else
#define new_nothrow new (std::nothrow)
#endif

#include "shared_operatingsystem_disable.h"

#endif