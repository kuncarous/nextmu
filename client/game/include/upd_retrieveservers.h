#ifndef __UPD_RETRIEVESERVERS_H__
#define __UPD_RETRIEVESERVERS_H__

#pragma once

#include "upd_base.h"

class NUpdateRetrieveServersTask : public NUpdateBaseTask
{
public:
    NUpdateRetrieveServersTask();
    
public:
    void Run() override;

private:
    void RunAsync();

private:
    std::future<void> Future;
};

#endif