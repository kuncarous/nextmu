#ifndef __MU_MODELRENDERER_H__
#define __MU_MODELRENDERER_H__

#pragma once

#include "mu_rendererconfig.h"

class NTerrain;
class NModel;

#pragma pack(4)
struct NModelViewSettings
{
	glm::mat4 Model;
	glm::mat4 ViewProj;
};

struct NModelSettings
{
	glm::vec4 LightPosition;
	glm::vec4 BodyLight;
	glm::vec4 BodyOrigin;
	mu_float BoneOffset;
	mu_float NormalScale;
	mu_float EnableLight;
	mu_float AlphaTest;
	mu_float PremultiplyAlpha;
	mu_float WorldTime;
	mu_float ZTestRef;
	mu_float BlendMeshLight;
	glm::vec2 BlendTexCoord;
	mu_float Dummy2;
	mu_float Dummy3;
};
#pragma pack()

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
		const NMeshRenderSettings *settings = nullptr,
		const NRenderVirtualMeshLightIndex *virtualMeshLights = nullptr
	);
	static void RenderBody(
		const NSkeletonInstance &skeleton,
		NModel *model,
		NRenderConfig &config,
		const NRenderVirtualMeshToggle *virtualMeshToggle = nullptr,
		const NRenderVirtualMeshLightIndex *virtualMeshLights = nullptr
	);
};

#endif