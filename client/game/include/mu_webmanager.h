#ifndef __MU_WEBMANAGER_H__
#define __MU_WEBMANAGER_H__

#pragma once

#include "web_requestbase.h"

namespace MUWebManager
{
    const mu_boolean Initialize();
	void Destroy();

	void Run();

	CURLMcode AddRequest(WEBRequestBasePtr request);
	void RemoveRequest(const mu_uuid requestId);
	void AbortRequest(WEBRequestBasePtr &request);
};

#endif