#ifndef __MU_ENVIRONMENT_CHARACTERS_H__
#define __MU_ENVIRONMENT_CHARACTERS_H__

#pragma once

#include "t_character_structs.h"
#include "res_item.h"
#include "mu_entity.h"

class NEnvironment;
class NCharacters
{
public:
	NCharacters(const NEnvironment *environment);

	const mu_boolean Initialize();
	void Destroy();

	void Update();
	void PreRender(const NRenderSettings &renderSettings);
	void Render(const NRenderSettings &renderSettings);

	void Clear();
	const entt::entity AddOrFind(
		const TCharacter::Settings object
	);
	void Remove(const entt::entity entity);

	void ClearAttachmentParts(const entt::entity entity);
	void AddAttachmentPartFromItem(const entt::entity entity, const NEntity::PartType partType, const NItemCategory category, const mu_uint16 index);
	void AddAttachmentPart(const entt::entity entity, const NEntity::PartType partType, NRender *render);
	void RemoveAttachmentPart(const entt::entity entity, const NEntity::PartType partType);

private:
	void MoveCharacter(const entt::entity);
	mu_boolean MovePath(NEntity::NPosition &position, NEntity::NMovement &movement, NEntity::NMoveSpeed &moveSpeed);

public:
	entt::registry &GetRegistry()
	{
		return Registry;
	}

private:
	const NEnvironment *Environment;
	entt::registry Registry;
	std::map<mu_key, entt::entity> RegistryMap;
};

#endif