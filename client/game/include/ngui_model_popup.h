#ifndef __NPOPUP_INSTANCE_H__
#define __NPOPUP_INSTANCE_H__

#pragma once

#include <NsGui/Enums.h>
#include "DelegateCommand.h"
#include "ngui_notifier.h"
#include "ngui_enums_popup.h"

class NGPopup : public NoesisGUI::NotifierBase
{
public:
	explicit NGPopup(
		EPopupID id,
		EPopupType type,
		EPopupPriority priority,
		mu_utf8string message,
		mu_utf8string accept = "Popup.Buttons.Accept",
		mu_utf8string cancel = "Popup.Buttons.Cancel"
	);

	const EPopupID GetId() const;
	const EPopupType GetType() const;
	const mu_char *GetTypeStr() const;
	const EPopupPriority GetPriority() const;
	const mu_char *GetMessage() const;
	const mu_char *GetAccept() const;
	const mu_char *GetCancel() const;

	NoesisApp::DelegateCommand *OnAccept() const;
	NoesisApp::DelegateCommand *OnCancel() const;

private:
	void OnAcceptReal(Noesis::BaseComponent *param) { OnAcceptClicked(param); }
	void OnCancelReal(Noesis::BaseComponent *param) { OnCancelClicked(param); }

protected:
	virtual void OnAcceptClicked(Noesis::BaseComponent *param);
	virtual void OnCancelClicked(Noesis::BaseComponent *param);

private:
	EPopupID Id;
	EPopupType Type;
	EPopupPriority Priority;
	Noesis::String Message;
	Noesis::String Accept;
	Noesis::String Cancel;

private:
	Noesis::Ptr<NoesisApp::DelegateCommand> OnAcceptDelegate;
	Noesis::Ptr<NoesisApp::DelegateCommand> OnCancelDelegate;

private:
	NS_DECLARE_REFLECTION(NGPopup, NoesisGUI::NotifierBase)
};

#endif