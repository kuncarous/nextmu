#ifndef __T_JOINT_ENUMS_H__
#define __T_JOINT_ENUMS_H__

#pragma once

#include <limits>

enum class JointType : mu_uint32
{
	Thunder1_V7,
	Invalid = std::numeric_limits<mu_uint32>::max(),
};

#endif