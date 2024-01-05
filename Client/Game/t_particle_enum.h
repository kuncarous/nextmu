#ifndef __T_PARTICLE_ENUMS_H__
#define __T_PARTICLE_ENUMS_H__

#pragma once

#include <limits>

enum class ParticleType : mu_uint32
{
	Effect_V0,
	Effect_V1,
	Effect_V2,
	Effect_V3,
	Effect_V4,
	Effect_V5,
	Effect_V6,
	Effect_V7,
	Smoke01_V0,
	Smoke05_V0,
	Smoke05_V1,
	TrueFire_Red_V5,
	Bubble_V0,
	Invalid = std::numeric_limits<mu_uint32>::max(),
};

#endif