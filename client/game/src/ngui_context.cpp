#include "mu_precompiled.h"
#include "ngui_context.h"

NGPopupContext *NGApplicationContext::GetPopup() const
{
	return Popup;
}

void NGApplicationContext::SetPopup(NGPopupContext *value)
{
	if (Popup == value) return;
	Popup.Reset(value);
	OnPropertyChanged("Popup");
}

NGUpdateContext *NGApplicationContext::GetUpdate() const
{
	return Update;
}

void NGApplicationContext::SetUpdate(NGUpdateContext *value)
{
    if (Update == value) return;
    Update.Reset(value);
    OnPropertyChanged("Update");
}

NGLoginContext *NGApplicationContext::GetLogin() const
{
	return Login;
}

void NGApplicationContext::SetLogin(NGLoginContext *value)
{
	if (Login == value) return;
	Login.Reset(value);
	OnPropertyChanged("Login");
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(NGApplicationContext, "ApplicationContext")
{
	NsImpl<Noesis::INotifyPropertyChanged>();
	NsProp("Popup", &NGApplicationContext::GetPopup);
	NsProp("Update", &NGApplicationContext::GetUpdate);
	NsProp("Login", &NGApplicationContext::GetLogin);
}

NS_END_COLD_REGION