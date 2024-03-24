#include "mu_precompiled.h"
#include "ngui_context_popup.h"

bool NGPopupContext::IsPopupVisible() const
{
	return CurrentPopup.GetPtr() != nullptr;
}

NGPopup *NGPopupContext::GetCurrentPopup() const
{
	return CurrentPopup;
}

void NGPopupContext::InsertPopup(Noesis::Ptr<NGPopup> popup)
{
	if (popup == nullptr) return;
	Popups[static_cast<mu_uint32>(popup->GetPriority())].push_back(popup);

	if (CurrentPopup == nullptr || popup->GetPriority() < CurrentPopup->GetPriority())
	{
		CurrentPopup = popup;
		OnPropertyChanged("IsPopupVisible");
		OnPropertyChanged("CurrentPopup");
	}
}

void NGPopupContext::RemovePopup(NGPopup *popup)
{
	if (popup == nullptr)
	{
		for (mu_uint32 priority = static_cast<mu_uint32>(EPopupPriority::ePopupCritical); priority < EPopupPriorityMax; ++priority)
		{
			auto &popups = Popups[priority];
			if (popups.empty() == true) continue;
			popups.erase(popups.begin());
			break;
		}
	}
	else
	{
		if (CurrentPopup != nullptr && popup == CurrentPopup.GetPtr())
		{
			CurrentPopup = nullptr;
			OnPropertyChanged("IsPopupVisible");
			OnPropertyChanged("CurrentPopup");
		}

		auto &popups = Popups[static_cast<mu_uint32>(popup->GetPriority())];
		for (auto iter = popups.begin(); iter != popups.end(); ++iter) {
			if (iter->GetPtr() != popup) continue;
			popups.erase(iter);
			break;
		}
	}

	for (mu_uint32 priority = static_cast<mu_uint32>(EPopupPriority::ePopupCritical); priority < EPopupPriorityMax; ++priority)
	{
		auto &popups = Popups[priority];
		if (popups.empty() == false)
		{
			CurrentPopup = popups[0];
			OnPropertyChanged("IsPopupVisible");
			OnPropertyChanged("CurrentPopup");
			break;
		}
	}
}

void NGPopupContext::RemovePopup(EPopupID id, EPopupPriority priority)
{
	if (CurrentPopup != nullptr && (CurrentPopup->GetId() == id && CurrentPopup->GetPriority() == priority))
	{
		CurrentPopup = nullptr;
		OnPropertyChanged("IsPopupVisible");
		OnPropertyChanged("CurrentPopup");
	}

	auto &popups = Popups[static_cast<mu_uint32>(priority)];
	for (auto iter = popups.begin(); iter != popups.end(); ++iter) {
		auto &popup = *iter;
		if (popup->GetId() != id) continue;
		popups.erase(iter);
		break;
	}

	for (mu_uint32 priority = static_cast<mu_uint32>(EPopupPriority::ePopupCritical); priority < EPopupPriorityMax; ++priority)
	{
		auto &popups = Popups[priority];
		if (popups.empty() == false)
		{
			CurrentPopup = popups[0];
			OnPropertyChanged("IsPopupVisible");
			OnPropertyChanged("CurrentPopup");
			break;
		}
	}
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(NGPopupContext)
{
	NsImpl<Noesis::INotifyPropertyChanged>();
	NsProp("IsPopupVisible", &NGPopupContext::IsPopupVisible);
	NsProp("CurrentPopup", &NGPopupContext::GetCurrentPopup);
}

NS_END_COLD_REGION