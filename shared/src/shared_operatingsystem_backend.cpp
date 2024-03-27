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
#if NEXTMU_CLIENT_SHARED == 1
				mu_utf8string filename;
				// Use reinterpret_cast to avoid compiler error due to two possible value_type sizes
                if constexpr (sizeof(std::filesystem::path::value_type) > 1u)
                {
                    filename = SDL_iconv_wchar_utf8(reinterpret_cast<const mu_unicode*>(iter->path().filename().c_str()));
                }
                else
				{
                    filename = reinterpret_cast<const mu_char*>(iter->path().filename().c_str());
                }
#else
                mu_utf8string filename = QString::fromWCharArray(iter->path().filename().c_str()).toUtf8().constData();
#endif

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
