#include "stdafx.h"
#include "ui_ultralight_filesystem.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
#include "ui_ultralight_mimetypes.inl"

namespace UIUltralight
{
	const mu_utf8string ResourcesPath = "resources/";

	RWFileSystem::~RWFileSystem()
	{
		for (auto &iter : Files)
		{
			SDL_RWclose(iter.second);
		}

		Files.clear();
	}

	///
	/// Check if file path exists, return true if exists.
	///
	bool RWFileSystem::FileExists(const ultralight::String16 &path)
	{
		return mu_rwexists<EGameDirectoryType::eSupport>(ResourcesPath + ConvertToUTF8String(path.data()));
	}

	///
	/// Get file size of previously opened file, store result in 'result'. Return true on success.
	///
	bool RWFileSystem::GetFileSize(ultralight::FileHandle handle, int64_t &result)
	{
		auto iter = Files.find(handle);
		if (iter == Files.end()) return false;
		result = SDL_RWsize(iter->second);
		return true;
	}

	///
	/// Get file mime type (eg "text/html"), store result in 'result'. Return true on success.
	///
	bool RWFileSystem::GetFileMimeType(const ultralight::String16 &path, ultralight::String16 &result)
	{
		const mu_utf8string filename = ConvertToUTF8String(path.data());
		const auto extensionAt = filename.find_last_of('.');
		if (extensionAt == mu_utf8string::npos || extensionAt + 1 >= filename.size()) return false;
		const mu_utf8string extension = filename.substr(extensionAt + 1);
		auto mimetype = MimeTypes.find(extension);
		if (mimetype == MimeTypes.end()) return false;
		result = ultralight::String16(mimetype->second.c_str());
		return true;
	}

	///
	/// Open file path for reading or writing. Return file handle on success, or invalidFileHandle on failure.
	///
	/// @NOTE:  As of this writing (v1.2), this function is only used for reading.
	///
	ultralight::FileHandle RWFileSystem::OpenFile(const ultralight::String16 &path, bool open_for_writing)
	{
		const mu_utf8string filename = ResourcesPath + ConvertToUTF8String(path.data());
		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, open_for_writing ? "wb" : "rb") == false)
		{
			return ultralight::invalidFileHandle;
		}

		auto handle = static_cast<ultralight::FileHandle>(Counter);
		Files.insert(std::pair(handle, fp));

		return handle;
	}

	///
	/// Close previously-opened file.
	///
	void RWFileSystem::CloseFile(ultralight::FileHandle &handle)
	{
		auto iter = Files.find(handle);
		if (iter != Files.end())
		{
			SDL_RWclose(iter->second);
			Files.erase(iter);
		}

		handle = ultralight::invalidFileHandle;
	}

	///
	/// Read from currently-opened file, return number of bytes read or -1 on failure.
	///
	int64_t RWFileSystem::ReadFromFile(ultralight::FileHandle handle, char *data, int64_t length)
	{
		auto iter = Files.find(handle);
		if (iter == Files.end()) return -1;

		size_t readedBytes = SDL_RWread(iter->second, data, 1, length);
		return static_cast<int64_t>(readedBytes);
	}
};
#endif