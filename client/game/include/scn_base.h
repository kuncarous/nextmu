#ifndef __SCN_BASE_H__
#define __SCN_BASE_H__

#pragma once

class NSceneBase
{
public:
    NSceneBase() {}
    virtual ~NSceneBase() {}

public:
    virtual mu_boolean Load() = 0;
    virtual void Unload() = 0;

public:
    virtual void Run() = 0;
};

typedef std::unique_ptr<NSceneBase> NSceneBasePtr;

#endif