#ifndef __MU_ENVIRONMENT_STRUCTS_H__
#define __MU_ENVIRONMENT_STRUCTS_H__

#pragma once

class NModel;
class NSkeletonInstance;

enum class EntityLightMode
{
	Terrain,
	Fixed,
	SinWorldTime,
};

NEXTMU_INLINE const EntityLightMode LightModeFromString(const mu_utf8string mode)
{
	if (mode == "terrain") return EntityLightMode::Terrain;
	if (mode == "fixed") return EntityLightMode::Fixed;
	if (mode == "sinworld") return EntityLightMode::SinWorldTime;
	return EntityLightMode::Terrain;
}

namespace MUObject
{
	struct LightSettings
	{
		EntityLightMode Mode;
		mu_boolean PrimaryLight;
		glm::vec3 Color;
		mu_float LightIntensity;
		mu_float TimeMultiplier;
		mu_float LightMultiplier;
		mu_float LightAdd;
	};

	struct Settings
	{
		mu_uint16 Type;
		NModel *Model;
		mu_boolean Renderable;
		mu_boolean Interactive;
		mu_boolean LightEnable;
		LightSettings Light;
		glm::vec3 Position;
		glm::vec3 Angle;
		mu_float Scale;
		glm::vec3 BBoxMin;
		glm::vec3 BBoxMax;
	};
};

namespace NEntity
{
	struct Attachment
	{
		const NModel *Model;
	};

	struct Renderable
	{};

	struct Interactive
	{};

	struct TerrainLight
	{
		glm::vec3 Color;
		mu_float Intensity;
		mu_boolean PrimaryLight;
	};

	struct FixedLight
	{
		glm::vec3 Color;
	};

	struct WorldTimeLight
	{
		mu_float TimeMultiplier;
		mu_float Multiplier;
		mu_float Add;
	};

	union LightSettings
	{
		TerrainLight Terrain;
		FixedLight Fixed;
		WorldTimeLight WorldTime;
	};

	struct Light
	{
		EntityLightMode Mode;
		LightSettings Settings;
	};

	struct RenderState
	{
		mu_boolean LightEnable; // true : calculate light using normals, false : apply body light directly
		glm::vec4 BodyLight;
	};

	struct Skeleton
	{
		mu_uint32 SkeletonOffset;
		NSkeletonInstance Instance;
	};

	struct Position // Rename
	{
		glm::vec3 Position;
		glm::vec3 Angle;
		mu_float Scale;
	};

	struct Animation
	{
		mu_uint16 CurrentAction = 0u;
		mu_uint16 PriorAction = 0u;
		mu_float CurrentFrame = 0.0f;
		mu_float PriorFrame = 0.0f;
	};
};

#endif