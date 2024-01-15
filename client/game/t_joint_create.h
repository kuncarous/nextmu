#ifndef __T_JOINT_CREATE_H__
#define __T_JOINT_CREATE_H__

#include "t_joint_enum.h"

struct NJointData
{
	mu_uint8 Layer = 0u;
	JointType Type;
	mu_uint16 SubType = 0u;
	glm::vec3 Position;
	glm::vec3 TargetPosition;
	glm::vec3 Angle = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Light = glm::vec3(1.0f, 1.0f, 1.0f);
	mu_float Scale = 10.0f;
};

#endif