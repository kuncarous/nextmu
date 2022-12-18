#ifndef __MU_MODELRENDERER_H__
#define __MU_MODELRENDERER_H__

#pragma once

class NTerrain;
class NModel;
class NSkeleton;

struct NModelRenderConfig
{
	const mu_uint32 BoneOffset;
	const glm::vec3 BodyOrigin;
	const mu_float BodyScale;
	const mu_boolean EnableLight;
	const glm::vec4 BodyLight;
};

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
		const NModelRenderConfig &config,
		const mu_uint32 transformCache
	);
	static void RenderBody(
		const NModel *model,
		const NModelRenderConfig &config
	);

private:
	static bgfx::UniformHandle TextureSampler;
	static bgfx::UniformHandle LightPositionUniform;
	static bgfx::UniformHandle Settings1Uniform;
	static bgfx::UniformHandle BodyLightUniform;
};

#endif