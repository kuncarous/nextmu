#ifndef __SHARED_OPERATINGSYSTEM_IO_H__
#define __SHARED_OPERATINGSYSTEM_IO_H__

#pragma once

#include <boost/filesystem.hpp>

extern mu_utf8string SupportPathUTF8;
extern mu_utf8string CachePathUTF8;
extern mu_utf8string UserPathUTF8;
extern mu_utf8string GameDataPathUTF8;

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

#if NEXTMU_CLIENT_SHARED == 1
typedef SDL_RWops* NFile;
typedef NFile* NFileRef;
#else
typedef std::unique_ptr<QFile> NFile;
typedef NFile& NFileRef;
#endif

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile_extstorage(NFileRef file, const mu_utf8string filename, const mu_char* mode)
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
    NFile file = nullptr;
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
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
#if NEXTMU_CLIENT_SHARED == 1
template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile_extstorage(NFileRef file, const mu_utf8string filename, const mu_char* mode)
#else
template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile_extstorage(NFileRef file, const mu_utf8string filename, const QIODeviceBase::OpenMode mode)
#endif
{
#if NEXTMU_CLIENT_SHARED == 1
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
#else
    if constexpr (dirType == EGameDirectoryType::eSupport)
    {
        file = std::make_unique<QFile>((SupportPathUTF8 + filename).c_str());
    }
    else if constexpr (dirType == EGameDirectoryType::eCache)
    {
        file = std::make_unique<QFile>((CachePathUTF8 + filename).c_str());
    }
    else if constexpr (dirType == EGameDirectoryType::eUser)
    {
        file = std::make_unique<QFile>((UserPathUTF8 + filename).c_str());
    }

    return file != nullptr && file->open(mode);
#endif
}

template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwexists_extstorage(const mu_utf8string filename)
{
    NFile file = nullptr;
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

    return tfile != nullptr;
}
#endif

#if NEXTMU_CLIENT_SHARED == 1
template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile(NFileRef file, const mu_utf8string filename, const mu_char* mode)
#else
template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwfromfile(NFileRef file, const mu_utf8string filename, const QIODeviceBase::OpenMode mode)
#endif
{
#if NEXTMU_CLIENT_SHARED == 1
#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
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
    *file = SDL_RWFromFile(filename.c_str(), mode);
#endif

    return *file != nullptr;
#else
    file = std::make_unique<QFile>(filename.c_str());
    return file != nullptr && file->open(mode);
#endif
}

template<const EGameDirectoryType dirType>
NEXTMU_INLINE const mu_boolean mu_rwexists(const mu_utf8string filename)
{
#if NEXTMU_CLIENT_SHARED == 1
#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
    if constexpr (dirType != EGameDirectoryType::eApplicationOnly)
    {
        if (mu_rwexists_extstorage<dirType>(filename) == true)
        {
            return true;
        }
    }

    NFile file = nullptr;
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
    NFile file = SDL_RWFromFile(filename.c_str(), "rb");

    if (file != nullptr)
    {
        SDL_RWclose(file);
    }
#endif

    return file != nullptr;
#else
    QString qfilename(filename.c_str());
    return QFileInfo::exists(qfilename) && QFileInfo(qfilename).isFile();
#endif
}

#if NEXTMU_CLIENT_SHARED == 1
NEXTMU_INLINE const mu_boolean mu_rwfromfile_swt(NFileRef file, const mu_utf8string filename, const mu_char* mode)
#else
NEXTMU_INLINE const mu_boolean mu_rwfromfile_swt(NFileRef file, const mu_utf8string filename, const QIODeviceBase::OpenMode mode)
#endif
{
    if (IsReadFromSupportAvailable() == true)
    {
        return mu_rwfromfile<EGameDirectoryType::eSupport>(file, filename, mode);
    }

    return mu_rwfromfile<EGameDirectoryType::eApplicationOnly>(file, filename, mode);
}

template<const EGameDirectoryType dirType>
NEXTMU_INLINE void MakeDirectory(mu_utf8string Path)
{
	std::replace(Path.begin(), Path.end(), '\\', '/');
#if ELION_OPERATING_SYSTEM_TYPE == ELION_OSTYPE_MOBILE
	if constexpr (dirType == EGameDirectoryType::eSupport)
	{
		Path = SupportPathUTF8 + Path;
	}
	else if constexpr (dirType == EGameDirectoryType::eCache)
	{
		Path = CachePathUTF8 + Path;
	}
	else if constexpr (dirType == EGameDirectoryType::eUser)
	{
		Path = UserPathUTF8 + Path;
	}
#endif

	boost::filesystem::path dirPath(Path);
	boost::system::error_code ec;
	boost::filesystem::create_directories(dirPath, ec);
}

#endif
