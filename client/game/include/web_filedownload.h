#ifndef __WEB_FILEDOWNLOAD_H__
#define __WEB_FILEDOWNLOAD_H__

#pragma once

#include "web_requestbase.h"

class WEBFileDownloadRequest : public WEBRequestBufferBase<>
{
public:
	typedef std::function<void(const WEBFileDownloadRequest *)> RequestCallbackFunc;
	typedef std::function<void(const mu_size &)> ReceivedDataCallbackFunc;

public:
	explicit WEBFileDownloadRequest(const mu_utf8string url, void *userData = nullptr, const WEBRequestMethodType methodType = WEBRequestMethodType::eGET) :
		WEBRequestBufferBase<>(methodType),
		Url(url),
		UserData(userData)
	{}
	virtual ~WEBFileDownloadRequest() {}

public:
	virtual void OnRequestAdded() override;
	virtual void OnRequestResponse(const mu_long responseCode) override;
	virtual void OnRequestFailed(const CURLcode errorCode, const mu_utf8string errorMessage) override;
	virtual void OnRequestAborted() override;
	virtual void OnReceivedData(const mu_size dataLength) override;

public:
	virtual mu_utf8string GetUrl() override;
	virtual curl_slist *GetHeaders() override;

	mu_uint8 *GetBuffer() const
	{
		return Buffer;
	}

	mu_size GetBufferSize() const
	{
		return BufferUsedSize;
	}

	void *GetUserData() const
	{
		return UserData;
	}

	void SetRequestAddedCallback(RequestCallbackFunc callback)
	{
		RequestAddedCallback = callback;
	}

	void SetRequestResponseCallback(RequestCallbackFunc callback)
	{
		RequestResponseCallback = callback;
	}

	void SetRequestFailedCallback(RequestCallbackFunc callback)
	{
		RequestFailedCallback = callback;
	}

	void SetRequestAbortedCallback(RequestCallbackFunc callback)
	{
		RequestAbortedCallback = callback;
	}

	void SetReceivedDataCallback(ReceivedDataCallbackFunc callback)
	{
		ReceivedDataCallback = callback;
	}

private:
    mu_utf8string Url;
	void *UserData;

public:
	CURLcode GetErrorCode() const
	{
		return ErrorCode;
	}

	mu_utf8string GetErrorMessage() const
	{
		return ErrorMessage;
	}

private:
	RequestCallbackFunc RequestAddedCallback;
	RequestCallbackFunc RequestResponseCallback;
	RequestCallbackFunc RequestFailedCallback;
	RequestCallbackFunc RequestAbortedCallback;
	ReceivedDataCallbackFunc ReceivedDataCallback;
	CURLcode ErrorCode = CURLE_OK;
	mu_utf8string ErrorMessage;
};

#endif