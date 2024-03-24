#ifndef __NCEF_CLIENT_H__
#define __NCEF_CLIENT_H__

#pragma once

#if NEXTMU_EMBEDDED_BROWSER == 1
#include <include/cef_client.h>
#include <include/wrapper/cef_helpers.h>

class NBrowserClient: public CefClient,
                     public CefLifeSpanHandler,
                     public CefLoadHandler
{
public:
    NBrowserClient(CefRefPtr<CefRenderHandler> ptr)
        : Handler(ptr)
    {
        assert(ptr != nullptr);
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override;

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual mu_boolean DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, mu_int32 httpStatusCode) override;
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, mu_boolean isLoading, mu_boolean canGoBack, mu_boolean canGoForward) override;

    // FIXME virtual override ?
    mu_boolean OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& failedUrl, CefString & errorText);
    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message) override {
        mu_info("Browser Message : {}", message->GetName().ToString());
		return false;
	}

public:
    mu_boolean IsCloseAllowed() const;
    mu_boolean IsLoaded() const;

private:
	mu_int32 BrowserId = -1;
    mu_atomic_bool Closing { false };
    mu_atomic_bool Loaded{false};
    CefRefPtr<CefRenderHandler> Handler = nullptr;

    IMPLEMENT_REFCOUNTING(NBrowserClient);
};
#endif

#endif