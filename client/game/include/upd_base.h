#ifndef __UPD_BASE_H__
#define __UPD_BASE_H__

#pragma once

#include <future>
#include "upd_enum.h"

class NUpdateBaseTask
{
public:
    NUpdateBaseTask(NUpdateState type) : Type(type) {}
    virtual ~NUpdateBaseTask() {}

public:
    virtual void Run() = 0;

private:
    NUpdateState Type;
};

typedef std::unique_ptr<NUpdateBaseTask> NUpdateBaseTaskPtr;

#endif