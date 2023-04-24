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

	void SetImmediateContext(Diligent::IDeviceContext *context);
	Diligent::IDeviceContext *GetImmediateContext();

	Diligent::IBuffer *GetProjUniform();
	Diligent::IBuffer *GetViewProjUniform();

	void SetViewTransform(cglm::mat4 view, cglm::mat4 projection);
	void GetViewProjection(cglm::mat4 dest);
	void GetProjection(cglm::mat4 dest);
	void GetView(cglm::mat4 dest);

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