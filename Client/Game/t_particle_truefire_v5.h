#ifndef __T_PARTICLE_TRUEFIRE_V5_H__
#define __T_PARTICLE_TRUEFIRE_V5_H__

#pragma once

#include "t_particle_base.h"

namespace TrueFireV5
{
	void Register(NInvokes &invokes);
	void Create(entt::registry &registry, const NCreateData &data);
	EnttIterator Move(entt::registry &registry, EnttIterator iter, EnttIterator last);
	EnttIterator Action(entt::registry &registry, EnttIterator iter, EnttIterator last);
	EnttIterator Render(entt::registry &registry, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer);
}

#endif