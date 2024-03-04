#ifndef __SCN_CHARACTERS_H__
#define __SCN_CHARACTERS_H__

#pragma once

#include "scn_base.h"

class NCharactersScene : public NSceneBase
{
public:
    virtual mu_boolean Load() override;
    virtual void Unload() override;

public:
    virtual void Run() override;
};

#endif