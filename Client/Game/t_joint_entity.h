#ifndef __T_JOINT_ENTITY_H__
#define __T_JOINT_ENTITY_H__

#pragma once

#include "t_joint_enum.h"
#include <boost/serialization/strong_typedef.hpp>

constexpr mu_uint32 MaxTails = 50;

namespace TJoint::Entity
{
	struct Info
	{
		mu_uint8 Layer;
		mu_uint8 SubType = 0u;
		JointType Type;
	};

	NEXTMU_INLINE mu_uint64 GetSort(const Info &data)
	{
		return (static_cast<mu_uint64>(data.Layer) << (sizeof(JointType) * 8llu)) | static_cast<mu_uint64>(data.Type);
	}

	BOOST_STRONG_TYPEDEF(mu_uint16, LifeTime);

	struct Position
	{
		glm::vec3 StartPosition;
		glm::vec3 Position;
		glm::vec3 TargetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Angle = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		mu_float Scale = 1.0f;
	};

	using Light = glm::vec4;

	typedef std::array<glm::vec3, 4> Tail;
	struct Tails
	{
		mu_int32 Begin;
		mu_int32 Count;
		mu_int32 MaxCount;
		std::array<Tail, MaxTails> Tails;
	};
}

#endif