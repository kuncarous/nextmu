#ifndef __T_PARTICLE_BASE_H__
#define __T_PARTICLE_BASE_H__

#pragma once

#include "t_particle_enum.h"
#include "t_particle_entity.h"
#include "t_particle_render.h"

struct NCreateData
{
	mu_uint8 Layer;
	ParticleType Type;
	glm::vec3 Position;
	glm::vec3 Angle;
	glm::vec3 Light;
	mu_float Scale;
};

typedef entt::internal::sparse_set_iterator<std::vector<entt::entity>> EnttIterator;
typedef std::function<void(entt::registry &, const NCreateData &)> NCreateFunc;
typedef std::function<EnttIterator(entt::registry &, EnttIterator, EnttIterator)> NMoveFunc;
typedef std::function<EnttIterator(entt::registry &, EnttIterator, EnttIterator)> NActionFunc;
typedef std::function<EnttIterator(entt::registry &, EnttIterator, EnttIterator, NRenderBuffer &renderBuffer)> NRenderFunc;

struct NInvokes
{
	std::map<ParticleType, NCreateFunc> Create;
	std::map<ParticleType, NMoveFunc> Move;
	std::map<ParticleType, NActionFunc> Action;
	std::map<ParticleType, NRenderFunc> Render;
};

#endif