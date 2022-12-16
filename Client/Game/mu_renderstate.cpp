#include "stdafx.h"
#include "mu_renderstate.h"
#include "mu_environment.h"

namespace MURenderState
{
	NEnvironment *Environment = nullptr;
	std::array<NTexture *, TextureAttachment::Count> Textures = { {} };

	void Reset()
	{
		Environment = nullptr;
		for (mu_uint32 n = 0; n < TextureAttachment::Count; ++n)
		{
			Textures[n] = nullptr;
		}
	}

	void AttachEnvironment(NEnvironment *environment)
	{
		Environment = environment;
	}

	void DetachEnvironment()
	{
		Environment = nullptr;
	}

	const NEnvironment *GetEnvironment()
	{
		return Environment;
	}

	const NTerrain *GetTerrain()
	{
		if (Environment == nullptr) return nullptr;
		return Environment->GetTerrain();
	}

	void AttachTexture(TextureAttachment::Type type, NTexture *texture)
	{
		Textures[type] = texture;
	}

	void DetachTexture(TextureAttachment::Type type)
	{
		Textures[type] = nullptr;
	}

	NTexture *GetTexture(TextureAttachment::Type type)
	{
		return Textures[type];
	}
}