#ifndef __MU_MODELRENDERER_H__
#define __MU_MODELRENDERER_H__

#pragma once

#include "mu_rendererconfig.h"

class NTerrain;
class NModel;
class NSkeleton;

class MUModelRenderer
{
protected:
	virtual ~MUModelRenderer() = 0;

public:
	static const mu_boolean Initialize();
	static void Destroy();

	static void RenderMesh(
		const NModel *model,
		const mu_uint32 meshIndex,
		const NRenderConfig &config,
		const mu_uint32 transformCache
	);
	static void RenderBody(
		const NSkeletonInstance &skeleton,
		const NModel *model,
		const NRenderConfig &config
	);

private:
	static bgfx::UniformHandle TextureSampler;
	static bgfx::UniformHandle LightPositionUniform;
	static bgfx::UniformHandle Settings1Uniform;
	static bgfx::UniformHandle BodyLightUniform;
};

#endif