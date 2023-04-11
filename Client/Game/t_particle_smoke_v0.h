#ifndef __T_PARTICLE_SMOKE_V0_H__
#define __T_PARTICLE_SMOKE_V0_H__

#pragma once

#include "t_particle_base.h"

namespace SmokeV0
{
	using namespace TParticle;
	void Register(NInvokes &invokes);
	void Create(entt::registry &registry, const NParticleData &data);
	EnttIterator Move(entt::registry &registry, EnttIterator iter, EnttIterator last);
	EnttIterator Action(entt::registry &registry, EnttIterator iter, EnttIterator last);
	EnttIterator Render(entt::registry &registry, EnttIterator iter, EnttIterator last, TParticle::NRenderBuffer &renderBuffer);
}

#endif