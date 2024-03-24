#ifndef __NPOPUP_CONTEXT_H__
#define __NPOPUP_CONTEXT_H__

#pragma once

#include <vector>
#include <NsGui/Enums.h>
#include "ngui_notifier.h"
#include "ngui_enums_popup.h"
#include "ngui_model_popup.h"

class NGPopupContext : public NoesisGUI::NotifierBase
{
public:
    bool IsPopupVisible() const;
    NGPopup *GetCurrentPopup() const;

public:
    void InsertPopup(Noesis::Ptr<NGPopup> popup);
	void RemovePopup(NGPopup *popup = nullptr);
	void RemovePopup(EPopupID id, EPopupPriority priority);

private:
    std::vector<Noesis::Ptr<NGPopup>> Popups[EPopupPriorityMax];
    Noesis::Ptr<NGPopup> CurrentPopup;

private:
	NS_DECLARE_REFLECTION(NGPopupContext, NoesisGUI::NotifierBase)
};

#endif