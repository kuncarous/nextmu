#include "shared_precompiled.h"

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
#include <filesystem>

namespace NXOperatingSystem
{
	void Initialize()
	{
	}

	const mu_boolean EnumerateFilesFromApplication(const mu_utf8string path, std::vector<mu_utf8string> &filesList)
	{
		filesList.clear();

		return true;
	}

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

		const mu_boolean removeDuplicates = filesList.size() > 0;
		std::filesystem::directory_iterator endIter;
		for (std::filesystem::directory_iterator iter(path, ec);
			 iter != endIter;
			 ++iter)
		{
			if (std::filesystem::is_regular_file(iter->status()) == true)
			{
				filesList.push_back(iter->path().filename());
			}
		}

		if (removeDuplicates == true)
		{
			std::sort(filesList.begin(), filesList.end());
			filesList.erase(std::unique(filesList.begin(), filesList.end()), filesList.end());
		}

		return true;
	}

    const mu_utf8string GetExternalStoragePath()
    {
        char *path = SDL_GetPrefPath(NEXTMU_COMPANY_NAME, NEXTMU_GAME_NAME);
        if (path == nullptr) return mu_utf8string();
        const mu_utf8string utf8Path(path);
        SDL_free(path);
        return utf8Path;
    }

	const mu_utf8string GetStorageSupportFilesPath()
	{
		return GetExternalStoragePath();
	}

	const mu_utf8string GetStorageCacheFilesPath()
	{
		return GetExternalStoragePath();
	}

	const mu_utf8string GetStorageUserFilesPath()
	{
		return GetExternalStoragePath();
	}
};
#endif
