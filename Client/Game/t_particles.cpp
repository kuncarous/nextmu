#include "stdafx.h"
#include "t_particles.h"
#include "t_particle_smoke_v0.h"
#include "t_particle_truefire_v5.h"

namespace TParticles
{
	NInvokes Invokes;

	void Register()
	{
		SmokeV0::Register(Invokes);
		TrueFireV5::Register(Invokes);
	}

	NCreateFunc GetCreate(const ParticleType type)
	{
		auto iter = Invokes.Create.find(type);
		if (iter == Invokes.Create.end()) return nullptr;
		return iter->second;
	}

	NMoveFunc GetMove(const ParticleType type)
	{
		auto iter = Invokes.Move.find(type);
		if (iter == Invokes.Move.end()) return nullptr;
		return iter->second;
	}

	NActionFunc GetAction(const ParticleType type)
	{
		auto iter = Invokes.Action.find(type);
		if (iter == Invokes.Action.end()) return nullptr;
		return iter->second;
	}

	NRenderFunc GetRender(const ParticleType type)
	{
		auto iter = Invokes.Render.find(type);
		if (iter == Invokes.Render.end()) return nullptr;
		return iter->second;
	}
}