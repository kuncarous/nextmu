#ifndef __MU_UPDATEMANAGER_H__
#define __MU_UPDATEMANAGER_H__

#pragma once

#include "web_filedownload.h"

namespace MUUpdateManager
{
    const mu_boolean Initialize();
    void Destroy();

	void WriteVersion(const mu_boolean requireVerify = false);
    mu_boolean Run();

    void Resume();
    void Pause();
};

#endif