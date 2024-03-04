#ifndef __SCN_GAME_H__
#define __SCN_GAME_H__

#pragma once

#include "scn_base.h"

class NGameScene : public NSceneBase
{
public:
    virtual mu_boolean Load() override;
    virtual void Unload() override;

public:
    virtual void Run() override;
};

#endif