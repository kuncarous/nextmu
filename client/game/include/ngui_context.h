#ifndef __NGUI_CONTEXT_H__
#define __NGUI_CONTEXT_H__

#pragma once

#include "ngui_notifier.h"
#include "ngui_context_popup.h"
#include "ngui_context_update.h"
#include "ngui_context_login.h"

class NGApplicationContext : public NoesisGUI::NotifierBase
{
public:
	NGPopupContext *GetPopup() const;
	void SetPopup(NGPopupContext *value);

	NGUpdateContext *GetUpdate() const;
	void SetUpdate(NGUpdateContext *value);

	NGLoginContext *GetLogin() const;
	void SetLogin(NGLoginContext *value);

private:
	Noesis::Ptr<NGPopupContext> Popup;
	Noesis::Ptr<NGUpdateContext> Update;
	Noesis::Ptr<NGLoginContext> Login;

private:
    NS_DECLARE_REFLECTION(NGApplicationContext, NoesisGUI::NotifierBase)
};

#endif