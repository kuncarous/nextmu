#ifndef __SHARED_OPERATINGSYSTEM_BACKEND_H__
#define __SHARED_OPERATINGSYSTEM_BACKEND_H__

#pragma once

#include "shared_operatingsystem_android.h"

namespace NXOperatingSystem
{
	extern mu_float ScreenScaleRatio;
	extern mu_boolean HasScreenKeyboardSupport;

	void Initialize();

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
	const mu_boolean GetDeviceScreenSize(mu_int32 &width, mu_int32 &height);
	const mu_boolean GetSystemLanguages(std::vector<mu_utf8string> &languages);
#endif

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
	const mu_boolean EnumerateFilesFromApplication(const mu_utf8string path, std::vector<mu_utf8string> &filesList);
#endif
	const mu_boolean EnumerateFiles(const mu_utf8string path, std::vector<mu_utf8string> &filesList);

	const mu_utf8string GetStorageSupportFilesPath();
	const mu_utf8string GetStorageCacheFilesPath();
	const mu_utf8string GetStorageUserFilesPath();

	template<typename T>
	NEXTMU_INLINE const T GetRealPixelSize(const T in)
	{
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
		return static_cast<T>(static_cast<mu_float>(in) * ScreenScaleRatio);
#else
		return in;
#endif
	}

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
	const mu_float GetThreadPriority();
	void SetThreadPriority(const mu_double priority);
	const mu_boolean HasScreenNotch();
#endif
};

#endif