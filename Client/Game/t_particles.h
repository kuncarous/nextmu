#ifndef __T_PARTICLES_H__
#define __T_PARTICLES_H__

#pragma once

#include "t_particle_base.h"

namespace TParticles
{
	void Register();
	NCreateFunc GetCreate(const ParticleType type);
	NMoveFunc GetMove(const ParticleType type);
	NActionFunc GetAction(const ParticleType type);
	NRenderFunc GetRender(const ParticleType type);
}

#endif