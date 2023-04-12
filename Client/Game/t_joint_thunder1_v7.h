#ifndef __T_JOINT_THUNDER1_V7_H__
#define __T_JOINT_THUNDER1_V7_H__

#pragma once

#include "t_joint_base.h"

namespace Thunder1V7
{
	using namespace TJoint;
	void Register(NInvokes &invokes);
	void Create(entt::registry &registry, const NJointData &data);
	EnttIterator Move(const EnttView &view, EnttIterator iter, EnttIterator last);
	EnttIterator Action(const EnttView &view, EnttIterator iter, EnttIterator last);
	EnttIterator Render(const EnttView &view, EnttIterator iter, EnttIterator last, TJoint::NRenderBuffer &renderBuffer);
}

#endif