#include "mu_precompiled.h"
#include "t_particle_base.h"

namespace TParticle
{
	std::map<mu_utf8string, ParticleType> Template::TemplateTypes;
	std::map<ParticleType, Template *> Template::Templates;

	void Initialize()
	{
		for (auto iter = Template::Templates.begin(); iter != Template::Templates.end(); ++iter)
		{
			iter->second->Initialize();
		}
	}

	std::optional<ParticleType> GetTemplateType(const mu_utf8string id)
	{
		auto iter = Template::TemplateTypes.find(id);
		if (iter == Template::TemplateTypes.end()) return std::nullopt;
		return iter->second;
	}

	Template* GetTemplate(ParticleType type)
	{
		auto iter = Template::Templates.find(type);
		if (iter == Template::Templates.end()) return nullptr;
		return iter->second;
	}
}