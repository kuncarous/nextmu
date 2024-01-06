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
	Flower01_V0,
	Flower02_V0,
	Flower03_V0,
	Flower01_V1,
	Flower02_V1,
	Flower03_V1,
	FlareBlue_V0,
	FlareBlue_V1,
	Flare02_V0,
	Smoke01_V0,
	Smoke05_V0,
	Smoke05_V1,
	TrueFire_Red_V5,
	Bubble_V0,
	Invalid = std::numeric_limits<mu_uint32>::max(),
};

#endif