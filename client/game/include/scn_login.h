#ifndef __SCN_LOGIN_H__
#define __SCN_LOGIN_H__

#pragma once

#include "scn_base.h"

class NLoginScene : public NSceneBase
{
public:
    virtual mu_boolean Load() override;
    virtual void Unload() override;

public:
    virtual void Run() override;
};

#endif