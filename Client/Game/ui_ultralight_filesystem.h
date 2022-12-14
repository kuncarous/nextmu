#ifndef __UI_ULTRALIGHT_FILESYSTEM_H__
#define __UI_ULTRALIGHT_FILESYSTEM_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
#include <Ultralight/Ultralight.h>

namespace UIUltralight
{
	class RWFileSystem : public ultralight::FileSystem
	{
	public:
		virtual ~RWFileSystem();

		///
		/// Check if file path exists, return true if exists.
		///
		virtual bool FileExists(const ultralight::String16 &path) override;

		///
		/// Get file size of previously opened file, store result in 'result'. Return true on success.
		///
		virtual bool GetFileSize(ultralight::FileHandle handle, int64_t &result) override;

		///
		/// Get file mime type (eg "text/html"), store result in 'result'. Return true on success.
		///
		virtual bool GetFileMimeType(const ultralight::String16 &path, ultralight::String16 &result) override;

		///
		/// Open file path for reading or writing. Return file handle on success, or invalidFileHandle on failure.
		///
		/// @NOTE:  As of this writing (v1.2), this function is only used for reading.
		///
		virtual ultralight::FileHandle OpenFile(const ultralight::String16 &path, bool open_for_writing) override;

		///
		/// Close previously-opened file.
		///
		virtual void CloseFile(ultralight::FileHandle &handle) override;

		///
		/// Read from currently-opened file, return number of bytes read or -1 on failure.
		///
		virtual int64_t ReadFromFile(ultralight::FileHandle handle, char *data, int64_t length) override;

	private:
		mu_uint32 Counter = 0;
		std::map<ultralight::FileHandle, SDL_RWops *> Files;
	};
};
#endif

#endif