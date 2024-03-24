#include "mu_precompiled.h"
#include "ncef_client.h"

#if NEXTMU_EMBEDDED_BROWSER == 1
CefRefPtr<CefLifeSpanHandler> NBrowserClient::GetLifeSpanHandler()
{
	return this;
}

CefRefPtr<CefLoadHandler> NBrowserClient::GetLoadHandler()
{
	return this;
}

CefRefPtr<CefRenderHandler> NBrowserClient::GetRenderHandler()
{
	return Handler;
}

// CefLifeSpanHandler methods.
void NBrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	// Must be executed on the UI thread.
	CEF_REQUIRE_UI_THREAD();

	BrowserId = browser->GetIdentifier();
}

mu_boolean NBrowserClient::DoClose(CefRefPtr<CefBrowser> browser)
{
	// Must be executed on the UI thread.
	CEF_REQUIRE_UI_THREAD();

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed description of this
	// process.
	if (browser->GetIdentifier() == BrowserId)
	{
		// Set a flag to indicate that the window close should be allowed.
		Closing = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void NBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
}

void NBrowserClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, mu_int32 httpStatusCode)
{
	std::cout << "OnLoadEnd(" << httpStatusCode << ")" << std::endl;
	Loaded = true;

	// Fix caret not appearing in input fields
	browser->GetHost()->SetFocus(true);
}

void NBrowserClient::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, mu_boolean isLoading, mu_boolean canGoBack, mu_boolean canGoForward)
{
	std::cout << "OnLoadingStateChange()" << std::endl;
}

// FIXME ?
mu_boolean NBrowserClient::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString &failedUrl, CefString &errorText)
{
	std::cout << "OnLoadError()" << std::endl;
	Loaded = true; //FIXME
	return true; //FIXME
}

void NBrowserClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
	std::cout << "OnLoadStart()" << std::endl;
}

mu_boolean NBrowserClient::IsCloseAllowed() const
{
	return Closing;
}

mu_boolean NBrowserClient::IsLoaded() const
{
	return Loaded;
}
#endif