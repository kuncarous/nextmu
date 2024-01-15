#ifndef __T_JOINT_TAILS_H__
#define __T_JOINT_TAILS_H__

#pragma once

#include "t_joint_entity.h"

NEXTMU_INLINE mu_int16 CalculateBeginTails(const mu_int16 beginTails, const mu_int16 maxTails)
{
	if (beginTails == 0) return maxTails - 1;
	return beginTails - 1;
}

NEXTMU_INLINE void CreateTail(TJoint::Entity::Tails &tails, const glm::vec3 &position, const glm::quat &rotation, mu_float scale)
{
	tails.Begin = CalculateBeginTails(tails.Begin, tails.MaxCount);
	const auto begin = tails.Begin;
	if (++tails.Count > tails.MaxCount - 1) tails.Count = tails.MaxCount - 1;

	scale *= 0.5f;
	auto &tail = tails.Tails[begin];
	tail[0] = position + (glm::vec3(-scale, 0.0f, 0.0f) * rotation);
	tail[1] = position + (glm::vec3(scale, 0.0f, 0.0f) * rotation);
	tail[2] = position + (glm::vec3(0.0f, 0.0f, -scale) * rotation);
	tail[3] = position + (glm::vec3(0.0f, 0.0f, scale) * rotation);
}

#endif