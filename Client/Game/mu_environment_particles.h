#ifndef __MU_ENVIRONMENT_PARTICLES_H__
#define __MU_ENVIRONMENT_PARTICLES_H__

#pragma once

#include "t_particle_base.h"
#include "t_particle_render.h"

class NParticles
{
public:
	const mu_boolean Initialize();
	void Destroy();

	void Create(
		const mu_uint8 layer,
		const ParticleType type,
		const glm::vec3 position,
		const glm::vec3 angle,
		const glm::vec3 light,
		const mu_float scale
	);
	void Update(const mu_uint32 updateCount);
	void Propagate();
	void Render();

private:
	entt::registry Registry;
	NRenderBuffer RenderBuffer;
	std::vector<NCreateData> PendingToCreate;
};

#endif