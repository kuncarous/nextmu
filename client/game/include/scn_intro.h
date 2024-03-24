#ifndef __SCN_INTRO_H__
#define __SCN_INTRO_H__

#pragma once

#include "scn_base.h"
#include "mu_resourcesmanager.h"
#include "ngui_context_update.h"

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
	NResourcesManagerPtr UpdateResources;
	NResourcesManagerPtr GameResources;
    Noesis::Ptr<NGUpdateContext> UpdateContext;
};

#endif