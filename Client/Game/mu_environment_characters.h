#ifndef __MU_ENVIRONMENT_CHARACTERS_H__
#define __MU_ENVIRONMENT_CHARACTERS_H__

#pragma once

#include "t_character_structs.h"
#include "res_item.h"
#include "mu_entity.h"

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

	void ClearAttachmentParts(const entt::entity entity);
	void AddAttachmentPartFromItem(const entt::entity entity, const NEntity::PartType partType, const NItemCategory category, const mu_uint16 index);
	void AddAttachmentPart(const entt::entity entity, const NEntity::PartType partType, const NRender *render);
	void RemoveAttachmentPart(const entt::entity entity, const NEntity::PartType partType);

private:
	entt::registry Registry;
	std::map<mu_key, entt::entity> RegistryMap;
};

#endif