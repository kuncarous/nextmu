#include "stdafx.h"
#include "mu_environment_characters.h"
#include "mu_environment.h"
#include "mu_entity.h"
#include "mu_camera.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "mu_state.h"
#include "mu_renderstate.h"
#include "mu_threadsmanager.h"

const mu_boolean NCharacters::Initialize()
{
	return true;
}

void NCharacters::Destroy()
{

}

void NCharacters::Update()
{
	const auto updateTime = MUState::GetUpdateTime();
	const auto frustum = MURenderState::GetCamera()->GetFrustum();
	const auto environment = MURenderState::GetEnvironment();
}

void NCharacters::Render()
{
}

void NCharacters::Clear()
{
	Registry.clear();
}

const entt::entity NCharacters::AddOrFind(
	const TCharacter::Settings character
)
{
	auto iter = RegistryMap.find(character.Key);
	if (iter != RegistryMap.end()) return iter->second;

	auto &registry = Registry;
	const auto entity = registry.create();
	RegistryMap.insert(std::pair(character.Key, entity));

	registry.emplace<NEntity::NIdentifier>(
		entity,
		character.Key
	);

	registry.emplace<NEntity::NRenderState>(
		entity,
		NEntity::NRenderState{}
	);

	registry.emplace<NEntity::NSkeleton>(
		entity,
		NEntity::NSkeleton{}
	);

	registry.emplace<NEntity::NPosition>(
		entity,
		NEntity::NPosition{
			.Position = glm::vec3(
				(static_cast<mu_float>(character.X) + 0.5f) * TerrainScale,
				(static_cast<mu_float>(character.Y) + 0.5f) * TerrainScale,
				0.0f
			),
			.Angle = glm::vec3(0.0f, 0.0f, character.Rotation),
		}
	);

	registry.emplace<NEntity::NAnimation>(
		entity,
		NEntity::NAnimation{}
	);

	return entity;
}

void NCharacters::Remove(const entt::entity entity)
{
	if (Registry.valid(entity) == false) return;
	const auto &identifier = Registry.get<NEntity::NIdentifier>(entity);
	RegistryMap.erase(identifier.Key);
	Registry.release(entity);
}