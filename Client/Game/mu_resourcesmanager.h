#ifndef __MU_RESOURCESMANAGER_H__
#define __MU_RESOURCESMANAGER_H__

#pragma once

#include "mu_resources.h"

class NModel;

namespace MUResourcesManager
{
	const mu_boolean Load();
	void Destroy();

	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, const mu_utf8string inputLayoutId);

	const mu_shader GetProgram(const mu_utf8string id);
	NGraphicsTexture *GetTexture(const mu_utf8string id);
	NModel *GetModel(const mu_utf8string id);
};

#endif