#include "stdafx.h"
#include "mu_environment.h"

void NEnvironment::ClearObjects()
{
	Objects.clear();
}

const entt::entity NEnvironment::AddObject(
	const MUObject::Settings object
)
{
	using namespace NEntity;

	auto &registry = Objects;
	const auto entity = Objects.create();
	registry.emplace<Attachment>(
		entity,
		object.Model
	);

	if (object.Renderable)
	{
		registry.emplace<Renderable>(entity);
	}

	if (object.Interactive)
	{
		registry.emplace<Interactive>(entity);
	}

	LightSettings lightSettings;
	switch (object.Light.Mode)
	{
	case EntityLightMode::Terrain:
		{
			lightSettings.Terrain.Color = object.Light.Color;
			lightSettings.Terrain.Intensity = object.Light.LightIntensity;
			lightSettings.Terrain.PrimaryLight = object.Light.PrimaryLight;
		}
		break;

	case EntityLightMode::Fixed:
		{
			lightSettings.Fixed.Color = object.Light.Color;
		}
		break;

	case EntityLightMode::SinWorldTime:
		{
			lightSettings.WorldTime.TimeMultiplier = object.Light.TimeMultiplier;
			lightSettings.WorldTime.Multiplier = object.Light.LightMultiplier;
			lightSettings.WorldTime.Add = object.Light.LightAdd;
		}
		break;
	}

	registry.emplace<Light>(
		entity,
		object.Light.Mode,
		lightSettings
	);

	registry.emplace<RenderState>(
		entity,
		object.LightEnable,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	
	NSkeletonInstance instance;
	instance.SetParent(
		object.Angle,
		glm::vec3(0.0f, 0.0f, 0.0f),
		1.0f
	);

	registry.emplace<Skeleton>(
		entity,
		NInvalidUInt32,
		instance
	);

	registry.emplace<Position>(
		entity,
		object.Position,
		object.Angle,
		object.Scale
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