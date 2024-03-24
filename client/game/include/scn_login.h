#ifndef __SCN_LOGIN_H__
#define __SCN_LOGIN_H__

#pragma once

#include "scn_base.h"
#include "mu_resourcesmanager.h"
#include "mu_environment.h"
#include "ngui_context_login.h"

class NLoginScene : public NSceneBase
{
public:
    NLoginScene(NResourcesManagerPtr manager) : NSceneBase(), GameResources(std::move(manager)) {}

public:
    virtual mu_boolean Load() override;
    virtual void Unload() override;

public:
    virtual void Run() override;

private:
    NResourcesManagerPtr GameResources;
	NEnvironmentPtr Environment;

	Noesis::Ptr<NGLoginContext> LoginContext;
};

#endif