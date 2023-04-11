#ifndef __T_JOINT_BASE_H__
#define __T_JOINT_BASE_H__

#pragma once

#include "t_joint_enum.h"
#include "t_joint_entity.h"
#include "t_joint_render.h"
#include "t_joint_create.h"

namespace TJoint
{
	typedef entt::internal::sparse_set_iterator<std::vector<entt::entity>> EnttIterator;
	typedef std::function<void(entt::registry &, const NJointData &)> NCreateFunc;
	typedef std::function<EnttIterator(entt::registry &, EnttIterator, EnttIterator)> NMoveFunc;
	typedef std::function<EnttIterator(entt::registry &, EnttIterator, EnttIterator)> NActionFunc;
	typedef std::function<EnttIterator(entt::registry &, EnttIterator, EnttIterator, TJoint::NRenderBuffer &renderBuffer)> NRenderFunc;

	struct NInvokes
	{
		std::map<JointType, NCreateFunc> Create;
		std::map<JointType, NMoveFunc> Move;
		std::map<JointType, NActionFunc> Action;
		std::map<JointType, NRenderFunc> Render;
	};
}

#endif