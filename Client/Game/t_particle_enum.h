#ifndef __MU_PARTICLES_ENUMS_H__
#define __MU_PARTICLES_ENUMS_H__

#pragma once

#include <limits>

enum class ParticleType : mu_uint32
{
	Smoke_V0,
	TrueFire_V5,
	Invalid = std::numeric_limits<mu_uint32>::max(),
};

#endif