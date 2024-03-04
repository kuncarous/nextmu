#ifndef __MU_RESOURCESMANAGER_H__
#define __MU_RESOURCESMANAGER_H__

#pragma once

class NModel;

namespace MUResourcesManager
{
    const mu_boolean Load();
	void Destroy();

	NModel *GetModel(const mu_utf8string id);
};

#endif
