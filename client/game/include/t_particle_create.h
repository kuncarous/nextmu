#ifndef __T_PARTICLE_CREATE_H__
#define __T_PARTICLE_CREATE_H__

#include "t_particle_enum.h"

struct NParticleData
{
	mu_uint8 Layer = 0u;
	ParticleType Type;
	mu_uint16 SubType = 0u;
	glm::vec3 Position;
	glm::vec3 Angle = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Light = glm::vec3(1.0f, 1.0f, 1.0f);
	mu_float Scale = 1.0f;
};

#endif