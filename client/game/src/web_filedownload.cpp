#include "mu_precompiled.h"
#include "web_filedownload.h"

void WEBFileDownloadRequest::OnRequestAdded()
{
}

void WEBFileDownloadRequest::OnRequestResponse(const mu_long responseCode)
{
	DestroyHeaders();

	if (!!RequestResponseCallback) RequestResponseCallback(this);
}

void WEBFileDownloadRequest::OnRequestFailed(const CURLcode errorCode, const mu_utf8string errorMessage)
{
	mu_debug_error("[WEBFileDownloadRequest] Request failed : ({}, {})", static_cast<mu_int32>(errorCode), errorMessage);
	ErrorCode = errorCode;
	ErrorMessage = errorMessage;

	DestroyBuffer();
	DestroyHeaders();

	if (!!RequestFailedCallback) RequestFailedCallback(this);
}

void WEBFileDownloadRequest::OnRequestAborted()
{
	DestroyBuffer();
	DestroyHeaders();

	if (!!RequestAbortedCallback) RequestAbortedCallback(this);
}

void WEBFileDownloadRequest::OnReceivedData(const mu_size dataLength)
{
	if (!!ReceivedDataCallback) ReceivedDataCallback(dataLength);
}

mu_utf8string WEBFileDownloadRequest::GetUrl()
{
	return Url;
}

curl_slist *WEBFileDownloadRequest::GetHeaders()
{
	DestroyHeaders();
	return Headers;
}