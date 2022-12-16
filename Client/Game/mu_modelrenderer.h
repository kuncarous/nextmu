#ifndef __MU_MODELRENDERER_H__
#define __MU_MODELRENDERER_H__

#pragma once

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

	static void RenderMesh(const NModel *model, const mu_uint32 bonesOffset, const mu_uint32 mesh, const mu_uint32 transformCache);
	static void RenderBody(const NModel *model, const mu_uint32 bonesOffset, const glm::vec3 bodyOrigin, const mu_float bodyScale);

private:
	static bgfx::UniformHandle TextureSampler;
	static bgfx::UniformHandle Settings1Uniform;
	static bgfx::ProgramHandle RenderProgram;
};

#endif