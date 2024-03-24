#ifndef __NGUI_CONTEXT_LOGIN_H__
#define __NGUI_CONTEXT_LOGIN_H__

#pragma once

#include <NsGui/Enums.h>
#include "DelegateCommand.h"
#include "ngui_notifier.h"
#include "upd_enum.h"

class NGLoginContext : public NoesisGUI::NotifierBase
{
public:
	NGLoginContext();

	bool GetShowLogin() const;
	void SetShowLogin(bool value);

	bool GetShowServers() const;
	void SetShowServers(bool value);

	NoesisApp::DelegateCommand *OnLogin() const;

private:
	void OnLoginClicked(Noesis::BaseComponent *param);

private:
	Noesis::Ptr<NoesisApp::DelegateCommand> OnLoginDelegate;
	bool ShowLogin = false;
	bool ShowServers = false;

private:
    NS_DECLARE_REFLECTION(NGLoginContext, NoesisGUI::NotifierBase)
};

#endif