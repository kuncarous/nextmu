#ifndef __MU_RENDERSTATE_H__
#define __MU_RENDERSTATE_H__

#pragma once

#include "mu_renderstate_enums.h"

class NEnvironment;
class NTerrain;
class NTexture;

namespace MURenderState
{
	void Reset();

	void AttachEnvironment(NEnvironment *environment);
	void DetachEnvironment();
	const NEnvironment *GetEnvironment();
	const NTerrain *GetTerrain();

	void AttachTexture(TextureAttachment::Type type, NTexture *texture);
	void DetachTexture(TextureAttachment::Type type);
	NTexture *GetTexture(TextureAttachment::Type type);
};

#endif