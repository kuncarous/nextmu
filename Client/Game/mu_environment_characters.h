#ifndef __MU_ENVIRONMENT_CHARACTERS_H__
#define __MU_ENVIRONMENT_CHARACTERS_H__

#pragma once

#include "t_character_structs.h"

class NCharacters
{
public:
	const mu_boolean Initialize();
	void Destroy();

	void Update();
	void Render();

	void Clear();
	const entt::entity AddOrFind(
		const TCharacter::Settings object
	);
	void Remove(const entt::entity entity);

private:
	entt::registry Registry;
	std::map<mu_key, entt::entity> RegistryMap;
};

#endif