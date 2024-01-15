#ifndef __SHARED_OPERATINGSYSTEM_IO_H__
#define __SHARED_OPERATINGSYSTEM_IO_H__

#pragma once

extern mu_utf8string SupportPathUTF8;
extern mu_unicodestring SupportPathUnicode;
extern mu_utf8string CachePathUTF8;
extern mu_unicodestring CachePathUnicode;
extern mu_utf8string UserPathUTF8;
extern mu_unicodestring UserPathUnicode;

enum class EGameDirectoryType : mu_uint32
{
	eApplicationOnly, // Application Path Only
	eSupport, // Game Data
	eCache, // Cache Data
	eUser, // User Data (available to user like game configuration)
	eRoot, // No Append Path
};

void SetReadFromSupport(const mu_boolean enable);
const mu_boolean IsReadFromSupportAvailable();

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile_extstorage(SDL_RWops** file, const mu_utf8string filename, const mu_char* mode)
{
	if constexpr (dirType == EGameDirectoryType::eSupport)
	{
		*file = SDL_RWFromFile((SupportPathUTF8 + filename).c_str(), mode);
	}
	else if constexpr (dirType == EGameDirectoryType::eCache)
	{
		*file = SDL_RWFromFile((CachePathUTF8 + filename).c_str(), mode);
	}
	else if constexpr (dirType == EGameDirectoryType::eUser)
	{
		*file = SDL_RWFromFile((UserPathUTF8 + filename).c_str(), mode);
	}

	return *file != nullptr;
}

template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwexists_extstorage(const mu_utf8string filename)
{
	SDL_RWops* file = nullptr;
	if constexpr (dirType == EGameDirectoryType::eSupport)
	{
		file = SDL_RWFromFile((SupportPathUTF8 + filename).c_str(), "rb");
	}
	else if constexpr (dirType == EGameDirectoryType::eCache)
	{
		file = SDL_RWFromFile((CachePathUTF8 + filename).c_str(), "rb");
	}
	else if constexpr (dirType == EGameDirectoryType::eUser)
	{
		file = SDL_RWFromFile((UserPathUTF8 + filename).c_str(), "rb");
	}

	if (file != nullptr)
	{
		SDL_RWclose(file);
	}

	return file != nullptr;
}
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile_extstorage(SDL_RWops** file, const mu_utf8string filename, const mu_char* mode)
{
	FILE* tfile = nullptr;
	if constexpr (dirType == EGameDirectoryType::eSupport)
	{
		tfile = fopen((SupportPathUTF8 + filename).c_str(), mode);
	}
	else if constexpr (dirType == EGameDirectoryType::eCache)
	{
		tfile = fopen((CachePathUTF8 + filename).c_str(), mode);
	}
	else if constexpr (dirType == EGameDirectoryType::eUser)
	{
		tfile = fopen((UserPathUTF8 + filename).c_str(), mode);
	}

	if (tfile == nullptr) return false;

	*file = SDL_RWFromFP(tfile, SDL_TRUE);
	return *file != nullptr;
}

template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwexists_extstorage(const mu_utf8string filename)
{
	FILE* tfile = nullptr;
	if constexpr (dirType == EGameDirectoryType::eSupport)
	{
		tfile = fopen((SupportPathUTF8 + filename).c_str(), "rb");
	}
	else if constexpr (dirType == EGameDirectoryType::eCache)
	{
		tfile = fopen((CachePathUTF8 + filename).c_str(), "rb");
	}
	else if constexpr (dirType == EGameDirectoryType::eUser)
	{
		tfile = fopen((UserPathUTF8 + filename).c_str(), "rb");
	}

	if (tfile != nullptr)
	{
		fclose(tfile);
	}

	return tfile != nullptr;
}
#endif

template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile(SDL_RWops** file, const mu_utf8string filename, const mu_char* mode)
{
#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
	if constexpr (dirType != EGameDirectoryType::eApplicationOnly)
	{
		if (mu_rwfromfile_extstorage<dirType>(file, filename, mode) == true)
		{
			return true;
		}
	}

	if constexpr (dirType == EGameDirectoryType::eApplicationOnly ||
		dirType == EGameDirectoryType::eSupport)
	{
		*file = SDL_RWFromFile(filename.c_str(), mode);
	}
#else
	* file = SDL_RWFromFile(filename.c_str(), mode);
#endif

	return *file != nullptr;
}

template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwexists(const mu_utf8string filename)
{
#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
	if constexpr (dirType != EGameDirectoryType::eApplicationOnly)
	{
		if (mu_rwexists_extstorage<dirType>(filename) == true)
		{
			return true;
		}
	}

	SDL_RWops* file = nullptr;
	if constexpr (dirType == EGameDirectoryType::eApplicationOnly ||
		dirType == EGameDirectoryType::eSupport)
	{
		file = SDL_RWFromFile(filename.c_str(), "rb");

		if (file != nullptr)
		{
			SDL_RWclose(file);
		}
	}
#else
	SDL_RWops* file = SDL_RWFromFile(filename.c_str(), "rb");

	if (file != nullptr)
	{
		SDL_RWclose(file);
	}
#endif

	return file != nullptr;
}

NEXTMU_INLINE const mu_boolean mu_rwfromfile_swt(SDL_RWops** file, const mu_utf8string filename, const mu_char* mode)
{
	if (IsReadFromSupportAvailable() == true)
	{
		return mu_rwfromfile<EGameDirectoryType::eSupport>(file, filename, mode);
	}

	return mu_rwfromfile<EGameDirectoryType::eApplicationOnly>(file, filename, mode);
}

#endif