#ifndef __MU_RENDERSTATE_H__
#define __MU_RENDERSTATE_H__

#pragma once

#include "mu_renderstate_enums.h"

class NCamera;
class NEnvironment;
class NTerrain;
class NTexture;

namespace MURenderState
{
	void Reset();

	void SetViewTransform(cglm::mat4 view, cglm::mat4 projection);
	void GetProjection(cglm::mat4 dest);
	void GetView(cglm::mat4 dest);

	void AttachCamera(NCamera *camera);
	void DetachCamera();
	void AttachEnvironment(NEnvironment *environment);
	void DetachEnvironment();

	const NCamera *GetCamera();
	const NEnvironment *GetEnvironment();
	const NTerrain *GetTerrain();

	void AttachTexture(TextureAttachment::Type type, const NTexture *texture);
	void DetachTexture(TextureAttachment::Type type);
	const NTexture *GetTexture(TextureAttachment::Type type);
};

#endif