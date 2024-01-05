#ifndef __T_PARTICLE_ENTITY_H__
#define __T_PARTICLE_ENTITY_H__

#pragma once

#include "t_particle_enum.h"
#include <boost/serialization/strong_typedef.hpp>

namespace TParticle::Entity
{
	struct Info
	{
		mu_uint8 Layer;
		mu_uint8 SubType = 0u;
		ParticleType Type;
	};

	NEXTMU_INLINE mu_uint64 GetSort(const Info &data)
	{
		return (static_cast<mu_uint64>(data.Layer) << (sizeof(ParticleType) * 8llu)) | static_cast<mu_uint64>(data.Type);
	}

	BOOST_STRONG_TYPEDEF(mu_uint16, LifeTime);
	BOOST_STRONG_TYPEDEF(mu_boolean, Trigger);

	struct Position
	{
		glm::vec3 StartPosition;
		glm::vec3 Position;
		glm::vec3 Angle = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		mu_float Scale = 1.0f;
	};

	using Light = glm::vec4;
	BOOST_STRONG_TYPEDEF(mu_float, Rotation);
	BOOST_STRONG_TYPEDEF(mu_float, Gravity);
	BOOST_STRONG_TYPEDEF(mu_uint8, Frame);

	BOOST_STRONG_TYPEDEF(mu_uint32, RenderGroup);
	BOOST_STRONG_TYPEDEF(mu_uint32, RenderIndex);
	BOOST_STRONG_TYPEDEF(mu_uint32, RenderCount);
}

#endif