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
	constexpr bgfx::ViewId RenderView = 240;
	void Reset();

	void AttachCamera(NCamera *camera);
	void DetachCamera();
	void AttachEnvironment(NEnvironment *environment);
	void DetachEnvironment();

	const NCamera *GetCamera();
	const NEnvironment *GetEnvironment();
	const NTerrain *GetTerrain();

	void AttachTexture(TextureAttachment::Type type, NTexture *texture);
	void DetachTexture(TextureAttachment::Type type);
	NTexture *GetTexture(TextureAttachment::Type type);
};

#endif