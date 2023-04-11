#ifndef __MU_ENVIRONMENT_JOINTS_H__
#define __MU_ENVIRONMENT_JOINTS_H__

#pragma once

#include "t_joint_create.h"
#include "t_joint_render.h"
#include "t_threading_helper.h"

class NJoints
{
public:
	const mu_boolean Initialize();
	void Destroy();

	void Create(const NJointData &data);
	void Update(const mu_uint32 updateCount);
	void Propagate();
	void Render();

private:
	entt::registry Registry;
	TJoint::NRenderBuffer RenderBuffer;
	std::vector<NJointData> PendingToCreate;
	std::vector<TThreadRange> ThreadsRange;
};

#endif