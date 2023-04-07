#ifndef __UI_NOESISGUI_STREAM_H__
#define __UI_NOESISGUI_STREAM_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	class Stream : public Noesis::Stream
	{
	public:
		static Noesis::Ptr<Stream> Load(const mu_utf8string filename, const mu_boolean loadMemory = false);

	private:
		Stream(SDL_RWops *file, const mu_boolean loadMemory);
		virtual ~Stream();

	public:
		/// Set the current position within the stream
		virtual void SetPosition(uint32_t pos) override;

		/// Returns the current position within the stream
		virtual uint32_t GetPosition() const override;

		/// Returns the length of the stream in bytes
		virtual uint32_t GetLength() const override;

		/// Reads data at the current position and advances it by the number of bytes read
		/// Returns the total number of bytes read. This can be less than the number of bytes requested
		virtual uint32_t Read(void* buffer, uint32_t size) override;

		void ReadToMemory();

		/// Returns the starting address for the whole data or null if not supported
		/// It is recommended, especially when reading fonts, to return a non-null value
		virtual const void* GetMemoryBase() const override;

		/// Closes the current stream and releases any resources associated with the current stream
		virtual void Close() override;

	private:
		SDL_RWops *File = nullptr;
		std::unique_ptr<mu_uint8[]> Memory;
	};
};
#endif

#endif