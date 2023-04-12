#include "stdafx.h"
#include "t_joints.h"
#include "t_joint_thunder1_v7.h"

namespace TJoints
{
	using namespace TJoint;
	NInvokes Invokes;

	void Register()
	{
		Thunder1V7::Register(Invokes);
	}

	NCreateFunc GetCreate(const JointType type)
	{
		auto iter = Invokes.Create.find(type);
		if (iter == Invokes.Create.end()) return nullptr;
		return iter->second;
	}

	NMoveFunc GetMove(const JointType type)
	{
		auto iter = Invokes.Move.find(type);
		if (iter == Invokes.Move.end()) return nullptr;
		return iter->second;
	}

	NActionFunc GetAction(const JointType type)
	{
		auto iter = Invokes.Action.find(type);
		if (iter == Invokes.Action.end()) return nullptr;
		return iter->second;
	}

	NRenderFunc GetRender(const JointType type)
	{
		auto iter = Invokes.Render.find(type);
		if (iter == Invokes.Render.end()) return nullptr;
		return iter->second;
	}

	NRenderGroupFunc GetRenderGroup(const JointType type)
	{
		auto iter = Invokes.RenderGroup.find(type);
		if (iter == Invokes.RenderGroup.end()) return nullptr;
		return iter->second;
	}
}