#ifndef __T_PARTICLE_TRUEFIRE_V5_H__
#define __T_PARTICLE_TRUEFIRE_V5_H__

#pragma once

#include "t_particle_base.h"

namespace TrueFireV5
{
	using namespace TParticle;
	void Register(NInvokes &invokes);
	void Create(entt::registry &registry, const NParticleData &data);
	EnttIterator Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last);
	EnttIterator Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last);
	EnttIterator Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, TParticle::NRenderBuffer &renderBuffer);
}

#endif