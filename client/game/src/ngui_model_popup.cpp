#include "mu_precompiled.h"
#include "ngui_model_popup.h"
#include "ui_noesisgui.h"
#include "ngui_context.h"

const mu_char *PopupTypes[EPopupTypeMax] = {
	"MessageOnly",
	"Accept",
	"AcceptCancel",
	"DestroyItem",
	"EnhanceItem",
	"DestroyMount",
	"EnhanceMount",
	"DestroyPet",
	"EnhancePet",
};

NGPopup::NGPopup(
	EPopupID id,
	EPopupType type,
	EPopupPriority priority,
	mu_utf8string message,
	mu_utf8string accept,
	mu_utf8string cancel
) : Id(id),
	Type(type),
	Priority(priority),
	Message(message.c_str()),
	Accept(accept.c_str()),
	Cancel(cancel.c_str())
{
	switch (type)
	{
	case EPopupType::eAccept:
		{
			OnAcceptDelegate = MakePtr<NoesisApp::DelegateCommand>(MakeDelegate(this, &NGPopup::OnAcceptReal));
		}
		break;

	case EPopupType::eAcceptCancel:
		{
			OnAcceptDelegate = MakePtr<NoesisApp::DelegateCommand>(MakeDelegate(this, &NGPopup::OnAcceptReal));
			OnCancelDelegate = MakePtr<NoesisApp::DelegateCommand>(MakeDelegate(this, &NGPopup::OnCancelReal));
		}
		break;

	default: break;
	}
}

const EPopupID NGPopup::GetId() const
{
	return Id;
}

const EPopupType NGPopup::GetType() const
{
	return Type;
}

const mu_char *NGPopup::GetTypeStr() const
{
	return PopupTypes[static_cast<mu_uint32>(Type)];
}

const EPopupPriority NGPopup::GetPriority() const
{
	return Priority;
}

const mu_char *NGPopup::GetMessage() const
{
	return Message.Str();
}

const mu_char *NGPopup::GetAccept() const
{
	return Accept.Str();
}

const mu_char *NGPopup::GetCancel() const
{
	return Cancel.Str();
}

NoesisApp::DelegateCommand *NGPopup::OnAccept() const
{
	return OnAcceptDelegate;
}

NoesisApp::DelegateCommand *NGPopup::OnCancel() const
{
	return OnCancelDelegate;
}

void NGPopup::OnAcceptClicked(Noesis::BaseComponent *param)
{
	UINoesis::GetContext()->GetPopup()->RemovePopup(this);
}

void NGPopup::OnCancelClicked(Noesis::BaseComponent *param)
{
	UINoesis::GetContext()->GetPopup()->RemovePopup(this);
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(NGPopup)
{
	NsImpl<Noesis::INotifyPropertyChanged>();
	NsProp("Type", &NGPopup::GetTypeStr);
	NsProp("Message", &NGPopup::GetMessage);
	NsProp("Accept", &NGPopup::GetAccept);
	NsProp("Cancel", &NGPopup::GetCancel);
	NsProp("OnAcceptClicked", &NGPopup::OnAccept);
	NsProp("OnCancelClicked", &NGPopup::OnCancel);
}

NS_END_COLD_REGION