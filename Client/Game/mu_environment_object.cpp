#include "stdafx.h"
#include "mu_environment.h"

void NEnvironment::ClearObjects()
{
	Objects.clear();
}

const entt::entity NEnvironment::AddObject(
	const NModel *model,
	const glm::vec3 light,
	const glm::vec3 position,
	const glm::vec3 angle,
	const mu_float scale
)
{
	using namespace NEntity;

	auto &registry = Objects;
	const auto entity = Objects.create();
	registry.emplace<Attachment>(
		entity,
		model
	);
	
	registry.emplace<Skeleton>(
		entity,
		NInvalidUInt32,
		NSkeletonInstance()
	);

	registry.emplace<Position>(
		entity,
		position,
		angle,
		scale
	);

	registry.emplace<Animation>(
		entity,
		static_cast<mu_uint16>(0u),
		static_cast<mu_uint16>(0u),
		0.0f,
		0.0f
	);

	return entity;
}

void NEnvironment::RemoveObject(const entt::entity entity)
{
	auto &registry = Objects;
	registry.destroy(entity);
}