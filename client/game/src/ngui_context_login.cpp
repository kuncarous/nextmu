#include "mu_precompiled.h"
#include "ngui_context_login.h"
#include "mu_browsermanager.h"
#include "mu_webservermanager.h"
#include "mu_sessionmanager.h"
#include <NsCore/Delegate.h>

NGLoginContext::NGLoginContext() : NoesisGUI::NotifierBase()
{
	OnLoginDelegate = MakePtr<NoesisApp::DelegateCommand>(MakeDelegate(this, &NGLoginContext::OnLoginClicked));
}

bool NGLoginContext::GetShowLogin() const
{
	return ShowLogin;
}

void NGLoginContext::SetShowLogin(bool value)
{
	if (ShowLogin == value) return;
	ShowLogin = value;
	OnPropertyChanged("ShowLogin");
}

bool NGLoginContext::GetShowServers() const
{
	return ShowServers;
}

void NGLoginContext::SetShowServers(bool value)
{
	if (ShowServers == value) return;
	ShowServers = value;
	OnPropertyChanged("ShowServers");
}

NoesisApp::DelegateCommand *NGLoginContext::OnLogin() const
{
	return OnLoginDelegate;
}

void NGLoginContext::OnLoginClicked(BaseComponent *param)
{
	if (ShowLogin == false) return;

#if NEXTMU_EMBEDDED_BROWSER == 1
	if (MUWebServerManager::StartListen() == false)
	{
		mu_error("Failed to start listen web server.");
		return;
	}

	if (MUBrowserManager::InitializeBrowser(fmt::format("{}{}?code={}", MUWebServerManager::GetBaseURL(), "auth/login", MUSessionManager::GenerateAuthCode()).c_str()) == false)
	{
		mu_error("Failed to initialize embedded browser.");
		return;
	}

	SetShowLogin(false);
#endif
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(NGLoginContext)
{
	NsImpl<Noesis::INotifyPropertyChanged>();
	NsProp("ShowLogin", &NGLoginContext::GetShowLogin);
	NsProp("ShowServers", &NGLoginContext::GetShowServers);
	NsProp("OnLoginClicked", &NGLoginContext::OnLogin);
}

NS_END_COLD_REGION