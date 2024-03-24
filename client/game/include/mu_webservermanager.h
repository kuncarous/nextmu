#ifndef __MU_WEBSERVERMANAGER_H__
#define __MU_WEBSERVERMANAGER_H__

#pragma once

#if NEXTMU_HTTP_SERVER == 1
namespace MUWebServerManager
{
    const mu_boolean Initialize();
    void Destroy();

    const mu_boolean StartListen();
    void StopListen();

    const mu_utf8string GetBaseURL();
};
#endif

#endif