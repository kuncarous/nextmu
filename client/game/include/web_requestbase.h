#ifndef __WEB_REQUESTBASE_H__
#define __WEB_REQUESTBASE_H__

#pragma once

#include <curl/curl.h>

NEXTMU_INLINE mu_boolean IsSuccessful(const mu_long responseCode)
{
	return responseCode >= 200 && responseCode < 300;
}

enum class WEBRequestState : mu_uint32
{
	Pending,
	Processing,
	Completed,
	Failed,
	Aborted,
};

enum class WEBRequestMethodType : mu_uint32
{
	eGET,
	ePOST,
	ePATCH,
	ePUT,
	eDELETE,
};

class WEBRequestBase
{
public:
	WEBRequestBase(
		const WEBRequestMethodType requestMethod
	) :
		Id(GenerateUUID()),
		RequestMethod(requestMethod),
		RequestState(WEBRequestState::Pending),
		ResponseCode(0),
		Headers(nullptr),
		Handle(nullptr)
	{}

	virtual ~WEBRequestBase()
	{
		DestroyHandle();
		DestroyHeaders();
	}

	virtual void OnRequestAdded() {}
	virtual void OnRequestResponse(const mu_long responseCode) {}
	virtual void OnRequestFailed(const CURLcode errorCode, const mu_utf8string errorMessage) {}
	virtual void OnRequestAborted() {}
	virtual void OnReceivedData(const mu_size dataLength) {};

	virtual mu_size OnWrite(mu_char *ptr, mu_size size, mu_size nmemb)
	{
		OnReceivedData(size * nmemb);
		return size * nmemb;
	}

public:
	const mu_uuid &GetId() const
	{
		return Id;
	}

	const WEBRequestMethodType GetRequestHttpType() const
	{
		return RequestMethod;
	}

	const WEBRequestState GetRequestState() const
	{
		return RequestState;
	}

	mu_long GetResponseCode() const
	{
		return ResponseCode;
	}

	virtual mu_utf8string GetUrl()
	{
		return "";
	}

	void SetHeaders(curl_slist *headers)
	{
		Headers = headers;
	}

	virtual curl_slist *GetHeaders()
	{
		return Headers;
	}

	void SetBody(const mu_utf8string body)
	{
		Body = body;
	}

	virtual const mu_char *GetBody()
	{
		return Body.c_str();
	}

	CURL *GetHandle() const
	{
		return Handle;
	}

public:
	void SetHandle(CURL *handle)
	{
		Handle = handle;
	}

	void SetRequestState(const WEBRequestState state)
	{
		RequestState = state;
	}

	void SetResponseCode(const mu_long responseCode)
	{
		ResponseCode = responseCode;
	}

public:
	void DestroyHandle()
	{
		if (Handle == nullptr) return;
		curl_easy_cleanup(Handle);
		Handle = nullptr;
	}

	void DestroyHeaders()
	{
		if (Headers == nullptr) return;
		curl_slist_free_all(Headers);
		Headers = nullptr;
	}

protected:
	mu_uuid Id;
	WEBRequestMethodType RequestMethod;
	WEBRequestState RequestState;
	mu_long ResponseCode;
	curl_slist *Headers;
	mu_utf8string Body;
	CURL *Handle;
};

template<const mu_uint32 BufferGrowSize = 64 * 1024>
class WEBRequestBufferBase : public WEBRequestBase
{
public:
	WEBRequestBufferBase(
		const WEBRequestMethodType requestMethod
	) :
		WEBRequestBase(requestMethod),
		Buffer(nullptr),
		BufferUsedSize(0),
		BufferMaxSize(0)
	{}

	virtual ~WEBRequestBufferBase()
	{
		DestroyHandle();
		DestroyBuffer();
	}

	virtual mu_size OnWrite(mu_char *ptr, mu_size size, mu_size nmemb) override
	{
		const mu_size needSize = size * nmemb;

		const mu_size bufferUsedSize = BufferUsedSize;
		const mu_size bufferMaxSize = BufferMaxSize;
		mu_uint8 *buffer = Buffer;

		if (bufferUsedSize + needSize > bufferMaxSize)
		{
			const mu_size diffSize = (bufferUsedSize + needSize) - bufferMaxSize;
			const mu_size newSize = bufferMaxSize + diffSize + (BufferGrowSize - diffSize % BufferGrowSize) % BufferGrowSize;

			buffer = (mu_uint8 *)mu_realloc(buffer, newSize);
			if (buffer == nullptr)
			{
				return 0;
			}

			BufferMaxSize = newSize;
			Buffer = buffer;
		}

		mu_memcpy(buffer + bufferUsedSize, ptr, needSize);
		BufferUsedSize += needSize;
		OnReceivedData(needSize);

		return needSize;
	}

public:
	void DestroyBuffer()
	{
		if (Buffer == nullptr) return;
		mu_free(Buffer);
		Buffer = nullptr;
		BufferUsedSize = 0;
		BufferMaxSize = 0;
	}

protected:
	mu_uint8 *Buffer;
	mu_size BufferUsedSize;
	mu_size BufferMaxSize;
};

typedef std::shared_ptr<WEBRequestBase> WEBRequestBasePtr;

#endif