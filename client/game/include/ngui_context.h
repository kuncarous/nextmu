#ifndef __NGUI_CONTEXT_H__
#define __NGUI_CONTEXT_H__

#pragma once

#include "ngui_notifier.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include "ngui_context_update.h"

class NGApplicationContext : public NoesisGUI::NotifierBase
{
public:
	NGUpdateContext *GetUpdate() const;
	void SetUpdate(NGUpdateContext *value);

private:
	Noesis::Ptr<NGUpdateContext> Update;

private:
    NS_DECLARE_REFLECTION(NGApplicationContext, NoesisGUI::NotifierBase)
};
#endif

#endif