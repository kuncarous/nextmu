#ifndef __T_PARTICLE_SMOKE_V0_H__
#define __T_PARTICLE_SMOKE_V0_H__

#pragma once

#include "t_particle_base.h"

namespace SmokeV0
{
	using namespace TParticle;
	void Register(NInvokes &invokes);
	void Create(entt::registry &registry, const NParticleData &data);
	EnttIterator Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last);
	EnttIterator Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last);
	EnttIterator Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer);
	void RenderGroup(const NRenderGroup &renderGroup, const NRenderBuffer &renderBuffer);
}

#endif