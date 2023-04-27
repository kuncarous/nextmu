#ifndef __MU_RENDERSTATE_H__
#define __MU_RENDERSTATE_H__

#pragma once

#include "mu_renderstate_enums.h"

class NCamera;
class NEnvironment;
class NTerrain;
class NGraphicsTexture;

namespace MURenderState
{
	const mu_boolean Initialize();
	void Destroy();
	void Reset();

	void SetRenderMode(const NRenderMode mode);
	const NRenderMode GetRenderMode();

	void SetShadowResourceId(const NResourceId resourceId);
	const NResourceId GetShadowResourceId();
	void SetShadowMode(const NShadowMode mode);
	const NShadowMode GetShadowMode();
	void SetShadowMap(Diligent::ShadowMapManager *shadowMap);
	Diligent::ShadowMapManager *GetShadowMap();

	Diligent::IBuffer *GetCameraUniform();
	Diligent::IBuffer *GetLightUniform();

	void SetViewTransform(glm::mat4 view, glm::mat4 projection, glm::mat4 frustumProjection);
	void SetViewProjection(glm::mat4 viewProj);
	glm::mat4 &GetViewProjection();
	glm::mat4 &GetFrustumProjection();
	glm::mat4 &GetProjection();
	glm::mat4 &GetView();

	void AttachCamera(NCamera *camera);
	void DetachCamera();
	void AttachEnvironment(NEnvironment *environment);
	void DetachEnvironment();

	const NCamera *GetCamera();
	const NEnvironment *GetEnvironment();
	NTerrain *GetTerrain();

	void AttachTexture(TextureAttachment::Type type, NGraphicsTexture *texture);
	void DetachTexture(TextureAttachment::Type type);
	NGraphicsTexture *GetTexture(TextureAttachment::Type type);
};

#endif