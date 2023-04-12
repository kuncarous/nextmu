#ifndef __T_PARTICLE_BASE_H__
#define __T_PARTICLE_BASE_H__

#pragma once

#include "t_particle_enum.h"
#include "t_particle_entity.h"
#include "t_particle_render.h"
#include "t_particle_create.h"

namespace TParticle
{
	template<typename... Other>
	using EnttViewType = entt::view<entt::get_t<Other...>>;

	typedef entt::registry EnttRegistry;
	typedef EnttViewType<TParticle::Entity::Info> EnttView;
	typedef EnttView::iterator EnttIterator;
	typedef std::function<void(EnttRegistry &, const NParticleData &)> NCreateFunc;
	typedef std::function<EnttIterator(EnttRegistry &, EnttView &, EnttIterator, EnttIterator)> NMoveFunc;
	typedef std::function<EnttIterator(EnttRegistry &, EnttView &, EnttIterator, EnttIterator)> NActionFunc;
	typedef std::function<EnttIterator(EnttRegistry &, EnttView &, EnttIterator, EnttIterator, TParticle::NRenderBuffer &renderBuffer)> NRenderFunc;

	struct NInvokes
	{
		std::map<ParticleType, NCreateFunc> Create;
		std::map<ParticleType, NMoveFunc> Move;
		std::map<ParticleType, NActionFunc> Action;
		std::map<ParticleType, NRenderFunc> Render;
	};
}

#endif