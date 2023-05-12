#ifndef __MU_MODELRENDERER_H__
#define __MU_MODELRENDERER_H__

#pragma once

#include "mu_rendererconfig.h"

class NTerrain;
class NModel;

class MUModelRenderer
{
protected:
	virtual ~MUModelRenderer() = 0;

public:
	static const mu_boolean Initialize();
	static void Destroy();

	static void Reset();
	static void RenderMesh(
		NModel *model,
		const mu_uint32 meshIndex,
		const NRenderConfig &config,
		const glm::mat4 modelMatrix,
		const NMeshRenderSettings *settings = nullptr
	);
	static void RenderBody(
		const NSkeletonInstance &skeleton,
		NModel *model,
		const NRenderConfig &config
	);
};

#endif