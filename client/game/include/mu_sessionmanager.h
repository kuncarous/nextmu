#ifndef __MU_SESSIONMANAGER_H__
#define __MU_SESSIONMANAGER_H__

#pragma once

#include <Poco/URI.h>

namespace MUSessionManager
{
    const mu_boolean Initialize(mu_utf8string url);
	const mu_utf8string GenerateAuthCode();
	const mu_boolean CheckAuthCode(const mu_utf8string code);
    const mu_utf8string GenerateAuthUrl();
    void ProcessAuthResponse(const Poco::URI &requestUri);
    void DestroySession();
};

#endif