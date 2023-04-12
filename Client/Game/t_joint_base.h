#ifndef __T_JOINT_BASE_H__
#define __T_JOINT_BASE_H__

#pragma once

#include "t_joint_config.h"
#include "t_joint_enum.h"
#include "t_joint_entity.h"
#include "t_joint_render.h"
#include "t_joint_create.h"
#include <type_traits>

namespace TJoint
{
	template<typename... Other>
	using EnttViewType = entt::view<entt::get_t<Other...>>;

	typedef entt::registry EnttRegistry;
	typedef EnttViewType<TJoint::Entity::Info> EnttView;
	typedef EnttView::iterator EnttIterator;
	typedef std::function<void(EnttRegistry &, const NJointData &)> NCreateFunc;
	typedef std::function<EnttIterator(EnttRegistry &, EnttView &, EnttIterator, EnttIterator)> NMoveFunc;
	typedef std::function<EnttIterator(EnttRegistry &, EnttView &, EnttIterator, EnttIterator)> NActionFunc;
	typedef std::function<EnttIterator(EnttRegistry &, EnttView &, EnttIterator, EnttIterator, NRenderBuffer &renderBuffer)> NRenderFunc;
	typedef std::function<void(const NRenderGroup &, const NRenderBuffer &)> NRenderGroupFunc;

	struct NInvokes
	{
		std::map<JointType, NCreateFunc> Create;
		std::map<JointType, NMoveFunc> Move;
		std::map<JointType, NActionFunc> Action;
		std::map<JointType, NRenderFunc> Render;
		std::map<JointType, NRenderGroupFunc> RenderGroup;
	};
}

#endif