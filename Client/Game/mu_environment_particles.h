#ifndef __MU_ENVIRONMENT_PARTICLES_H__
#define __MU_ENVIRONMENT_PARTICLES_H__

#pragma once

#include "t_particle_create.h"
#include "t_particle_render.h"

class NParticles
{
public:
	const mu_boolean Initialize();
	void Destroy();

	void Create(const NParticleData &data);
	void Update();
	void Propagate();
	void Render();

private:
	entt::registry Registry;
	TParticle::NRenderBuffer RenderBuffer;
	std::vector<NParticleData> PendingToCreate;
};

#endif