#ifndef __SCN_INTRO_H__
#define __SCN_INTRO_H__

#pragma once

#include "scn_base.h"
#include "mu_resourcesmanager.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include "ngui_context_update.h"
#endif

class NIntroScene : public NSceneBase
{
public:
    virtual mu_boolean Load() override;
    virtual void Unload() override;

public:
    virtual void Run() override;

public:
    mu_boolean FinishedUpdate = false;
    mu_boolean CanStartGame = false;
	std::unique_ptr<NResourcesManager> UpdateResources;
	std::unique_ptr<NResourcesManager> GameResources;

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
    Noesis::Ptr<NGUpdateContext> UpdateContext;
#endif
};

#endif