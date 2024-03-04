#include "mu_precompiled.h"
#include "mu_webmanager.h"
#include <curl/curl.h>

namespace MUWebManager
{
	CURLM *CurlMultiHandle = nullptr;
	std::map<mu_utf8string, WEBRequestBasePtr> Requests;
	std::map<mu_size, mu_utf8string> HandlesMap;
	mu_atomic_int32_t RunningRequests = 0;

    const mu_boolean Initialize()
    {
        if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
        {
            mu_error("Failed to initialize cURL.");
            return false;
		}

		CurlMultiHandle = curl_multi_init();
		if (CurlMultiHandle == nullptr)
		{
			return false;
		}

		return true;
    }

    void Destroy()
	{
		for (auto &[id, request] : Requests)
		{
			AbortRequest(request);
		}
		Requests.clear();

		if (CurlMultiHandle != nullptr)
		{
			curl_multi_cleanup(CurlMultiHandle);
			CurlMultiHandle = nullptr;
		}

	    curl_global_cleanup();
	}

	void Run()
	{
		const mu_int32 runningCount = static_cast<mu_int32>(HandlesMap.size());
		if (runningCount < 1) return;

		mu_int32 runningHandles = 0;
		CURLMcode result = curl_multi_perform(CurlMultiHandle, &runningHandles);
		if (result != CURLM_OK)
		{
			if (result == CURLM_CALL_MULTI_PERFORM) return;

			mu_debug_error("Curl multi perform failed : {}", static_cast<mu_int32>(result));
			return;
		}

		if (runningHandles != runningCount)
		{
			for (;;)
			{
				mu_int32 msgq = 0;
				CURLMsg *cm = curl_multi_info_read(CurlMultiHandle, &msgq);

				if (cm == nullptr)
				{
					break;
				}

				auto handleIter = HandlesMap.find(reinterpret_cast<mu_size>(cm->easy_handle));
				if (handleIter == HandlesMap.end()) continue;
				const mu_utf8string requestId = handleIter->second;
				HandlesMap.erase(handleIter);

				auto requestIter = Requests.find(requestId);
				if (requestIter == Requests.end()) continue;
				WEBRequestBasePtr &request = requestIter->second;

				CURLcode curlCode = cm->data.result;
				mu_long responseCode = 0;
				if (
					curlCode != CURLE_OK ||
					cm->msg != CURLMSG_DONE ||
					(curlCode = curl_easy_getinfo(cm->easy_handle, CURLINFO_RESPONSE_CODE, &responseCode)) != CURLE_OK
					)
				{
					request->SetRequestState(WEBRequestState::Failed);
					const char *error = curl_easy_strerror(curlCode);
					request->OnRequestFailed(curlCode, error);
				}
				else
				{
					request->SetRequestState(WEBRequestState::Completed);
					request->SetResponseCode(responseCode);
					request->OnRequestResponse(responseCode);
				}

				request->DestroyHandle();
				Requests.erase(requestIter);
			}
		}
	}

	mu_size WriteCallback(mu_char *ptr, mu_size size, mu_size nmemb, void *userdata)
	{
		return reinterpret_cast<WEBRequestBase *>(userdata)->OnWrite(ptr, size, nmemb);
	}

	CURLMcode AddRequest(WEBRequestBasePtr request)
	{
		CURL *handle = curl_easy_init();
		if (handle == nullptr)
		{
			mu_debug_error("Curl handle is null");
			return CURLM_BAD_EASY_HANDLE;
		}

		switch (request->GetRequestHttpType())
		{
		case WEBRequestMethodType::ePOST:
			{
				curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "POST");
				curl_easy_setopt(handle, CURLOPT_POSTFIELDS, request->GetBody());
			}
			break;

		case WEBRequestMethodType::ePUT:
			{
				curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PUT");
				curl_easy_setopt(handle, CURLOPT_POSTFIELDS, request->GetBody());
			}
			break;

		case WEBRequestMethodType::ePATCH:
			{
				curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PATCH");
				curl_easy_setopt(handle, CURLOPT_POSTFIELDS, request->GetBody());
			}
			break;
		}

		curl_easy_setopt(handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(handle, CURLOPT_CAINFO, "cacert.pem");
		//curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(handle, CURLOPT_URL, request->GetUrl().c_str());
		curl_easy_setopt(handle, CURLOPT_HTTPHEADER, request->GetHeaders());
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, request.get());

		CURLMcode result = curl_multi_add_handle(CurlMultiHandle, handle);
		if (result != CURLM_OK)
		{
			curl_easy_cleanup(handle);
			return result;
		}

		request->SetRequestState(WEBRequestState::Processing);
		request->OnRequestAdded();

		const mu_utf8string requestId = request->GetId().toString();
		request->SetHandle(handle);
		HandlesMap.insert(std::make_pair(reinterpret_cast<mu_size>(request->GetHandle()), requestId));
		Requests.insert(std::make_pair(requestId, std::move(request)));

		return result;
	}

	void RemoveRequest(const mu_uuid requestId)
	{
		auto iter = Requests.find(requestId.toString());
		if (iter == Requests.end()) return;

		WEBRequestBasePtr &request = iter->second;
		AbortRequest(request);

		Requests.erase(iter);
	}

	void AbortRequest(WEBRequestBasePtr &request)
	{
		if (request->GetHandle() == nullptr) return;
		curl_multi_remove_handle(CurlMultiHandle, request->GetHandle());
		HandlesMap.erase(reinterpret_cast<mu_size>(request->GetHandle()));
		request->DestroyHandle();
		switch (request->GetRequestState())
		{
		case WEBRequestState::Pending:
		case WEBRequestState::Processing:
			request->SetRequestState(WEBRequestState::Aborted);
			request->OnRequestAborted();
			break;
		}
	}
};