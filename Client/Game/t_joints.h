#ifndef __T_JOINTS_H__
#define __T_JOINTS_H__

#pragma once

#include "t_joint_base.h"

namespace TJoints
{
	using namespace TJoint;
	void Register();
	NCreateFunc GetCreate(const JointType type);
	NMoveFunc GetMove(const JointType type);
	NActionFunc GetAction(const JointType type);
	NRenderFunc GetRender(const JointType type);
	NRenderGroupFunc GetRenderGroup(const JointType type);
}

#endif