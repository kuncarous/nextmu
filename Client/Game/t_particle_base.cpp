#include "stdafx.h"
#include "t_particle_base.h"

namespace TParticle
{
	std::map<ParticleType, Template *> Template::Templates;

	Template *GetTemplate(ParticleType type)
	{
		auto iter = Template::Templates.find(type);
		if (iter == Template::Templates.end()) return nullptr;
		return iter->second;
	}
}