#include "stdafx.h"
#include "mu_environment.h"

void NEnvironment::ClearCharacters()
{
	Characters.clear();
}

const entt::entity NEnvironment::AddOrFindCharacter(
	const MUCharacter::Settings character
)
{
	using namespace NEntity;

	auto iter = CharactersMap.find(character.Key);
	if (iter != CharactersMap.end()) return iter->second;

	const auto entity = Characters.create();
	CharactersMap.insert(std::pair(character.Key, entity));

	Characters.emplace<Identifier>(
		entity,
		character.Key
	);

	Characters.emplace<RenderState>(
		entity,
		RenderFlags(),
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	);

	Characters.emplace<Skeleton>(
		entity,
		NInvalidUInt32,
		NSkeletonInstance()
	);

	Characters.emplace<Position>(
		entity,
		glm::vec3(
			(static_cast<mu_float>(character.X) + 0.5f) * TerrainScale,
			(static_cast<mu_float>(character.Y) + 0.5f) * TerrainScale,
			0.0f
		),
		glm::vec3(0.0f, 0.0f, character.Rotation),
		1.0f
	);

	Characters.emplace<Animation>(
		entity,
		0u,
		0u,
		0.0f,
		0.0f
	);

	return entity;
}

void NEnvironment::RemoveCharacter(const entt::entity entity)
{
	if (Characters.valid(entity) == false) return;
	const auto &identifier = Characters.get<NEntity::Identifier>(entity);
	CharactersMap.erase(identifier.Key);
	Characters.release(entity);
}