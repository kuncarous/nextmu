#include "shared_precompiled.h"

namespace NXOperatingSystem
{
	mu_float ScreenScaleRatio = 1.0f;
	mu_boolean HasScreenKeyboardSupport = false;

#if NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_ANDROID && \
	NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_IOS && \
	NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_MACOS
	void Initialize()
	{

	}
#endif

#if NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_ANDROID && \
	NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_IOS && \
	NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_MACOS
	const mu_boolean EnumerateFiles(const mu_utf8string path, std::vector<mu_utf8string> &filesList)
	{
		std::filesystem::path pathHandler(path);
		filesList.clear();

		std::error_code ec;
		if (std::filesystem::exists(pathHandler, ec) == false)
		{
			return false;
		}

		if (std::filesystem::is_directory(path, ec) == false)
		{
			return false;
		}

		std::filesystem::directory_iterator endIter;
		for (std::filesystem::directory_iterator iter(path, ec);
			 iter != endIter;
			 ++iter)
		{
			if (std::filesystem::is_regular_file(iter->status()) == true)
			{
				mu_utf8string filename = SDL_iconv_wchar_utf8(iter->path().filename().c_str());

				filesList.push_back(filename);
			}
		}

		return true;
	}
#endif

#if NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_ANDROID && \
	NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_IOS && \
	NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_MACOS
	const mu_utf8string GetStorageSupportFilesPath()
	{
		return mu_utf8string();
	}

	const mu_utf8string GetStorageUserFilesPath()
	{
		return mu_utf8string();
	}
#endif
};