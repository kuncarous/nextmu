#ifndef __T_JOINT_BASE_H__
#define __T_JOINT_BASE_H__

#pragma once

#include "t_joint_enum.h"
#include "t_joint_entity.h"
#include "t_joint_render.h"
#include "t_joint_create.h"
#include <type_traits>

namespace TJoint
{
	template<typename... Other>
	using EnttViewType = entt::view<entt::get_t<Other...>>;

	typedef EnttViewType<JOINT_VIEW> EnttView;
	typedef EnttView::iterator EnttIterator;
	typedef std::function<void(entt::registry &, const NJointData &)> NCreateFunc;
	typedef std::function<EnttIterator(const EnttView &, EnttIterator, EnttIterator)> NMoveFunc;
	typedef std::function<EnttIterator(const EnttView &, EnttIterator, EnttIterator)> NActionFunc;
	typedef std::function<EnttIterator(const EnttView &, EnttIterator, EnttIterator, TJoint::NRenderBuffer &renderBuffer)> NRenderFunc;

	struct NInvokes
	{
		std::map<JointType, NCreateFunc> Create;
		std::map<JointType, NMoveFunc> Move;
		std::map<JointType, NActionFunc> Action;
		std::map<JointType, NRenderFunc> Render;
	};
}

#endif