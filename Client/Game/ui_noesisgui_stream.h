#ifndef __UI_NOESISGUI_STREAM_H__
#define __UI_NOESISGUI_STREAM_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	class Stream : public Noesis::Stream
	{
	public:
		static Noesis::Ptr<Stream> Load(const mu_utf8string filename);

	private:
		Stream(SDL_RWops *file);
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
		virtual uint32_t Read(void *buffer, uint32_t size) override;

		/// Closes the current stream and releases any resources associated with the current stream
		virtual void Close() override;

	private:
		SDL_RWops *File = nullptr;
	};
};
#endif

#endif