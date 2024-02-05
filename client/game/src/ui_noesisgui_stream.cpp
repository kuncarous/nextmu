#include "mu_precompiled.h"
#include "ui_noesisgui_stream.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	Noesis::Ptr<Stream> Stream::Load(const mu_utf8string filename, const mu_boolean loadMemory)
	{
		SDL_RWops *file = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&file, filename, "rb") == false)
		{
			return nullptr;
		}

		return Noesis::Ptr<Stream>(new Stream(filename, file, loadMemory));
	}

	Stream::Stream(const mu_utf8string filename, SDL_RWops *file, const mu_boolean loadMemory) : Filename(filename), File(file)
	{
		if (loadMemory) ReadToMemory();
	}

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

	void Stream::ReadToMemory()
	{
		const auto offset = SDL_RWtell(File);
		if (offset == -1) return;
		const auto fileSize = SDL_RWsize(File);
		if (fileSize == -1) return;
		std::unique_ptr<mu_uint8[]> data(new_nothrow mu_uint8[fileSize]);
		if (!data) return;
		SDL_RWseek(File, 0, RW_SEEK_SET);
		const auto readSize = SDL_RWread(File, data.get(), 1, fileSize);
		SDL_RWseek(File, offset, RW_SEEK_SET);
		if (readSize != fileSize) return;
		Memory.swap(data);
	}

	const void* Stream::GetMemoryBase() const
	{
		return Memory.get();
	}

	void Stream::Close()
	{
		if (File == nullptr) return;
		SDL_RWclose(File);
		File = nullptr;
		Memory.reset();
	}
};
#endif