#include "stdafx.h"
#include "ui_noesisgui_stream.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	Noesis::Ptr<Stream> Stream::Load(const mu_utf8string filename)
	{
		SDL_RWops *file = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&file, filename, "rb") == false)
		{
			return nullptr;
		}

		return Noesis::Ptr<Stream>(new Stream(file));
	}

	Stream::Stream(SDL_RWops *file) : File(file)
	{}

	Stream::~Stream()
	{
		Close();
	}

	void Stream::SetPosition(uint32_t pos)
	{
		if (File == nullptr) return;
		SDL_RWseek(File, static_cast<Sint64>(pos), RW_SEEK_SET);
	}

	uint32_t Stream::GetPosition() const
	{
		if (File == nullptr) return 0u;
		return static_cast<uint32_t>(SDL_RWtell(File));
	}

	uint32_t Stream::GetLength() const
	{
		if (File == nullptr) return 0u;
		return static_cast<uint32_t>(SDL_RWsize(File));
	}

	uint32_t Stream::Read(void *buffer, uint32_t size)
	{
		if (File == nullptr) return 0u;
		return static_cast<uint32_t>(SDL_RWread(File, buffer, 1, size));
	}

	void Stream::Close()
	{
		if (File == nullptr) return;
		SDL_RWclose(File);
		File = nullptr;
	}
};
#endif