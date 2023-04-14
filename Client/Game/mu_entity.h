#ifndef __MU_ENTITY_H__
#define __MU_ENTITY_H__

#pragma once

#include "mu_entity_light.h"

class NModel;
class NSkeletonInstance;
namespace NEntity
{
	struct NIdentifier
	{
		const mu_key Key;
	};

	struct NRenderable
	{};

	struct NInteractive
	{};

	struct NTerrainLight
	{
		glm::vec3 Color;
		mu_float Intensity;
		mu_boolean PrimaryLight;
	};

	struct NFixedLight
	{
		glm::vec3 Color;
	};

	struct NWorldTimeLight
	{
		mu_float TimeMultiplier;
		mu_float Multiplier;
		mu_float Add;
	};

	union NLightSettings
	{
		NTerrainLight Terrain;
		NFixedLight Fixed;
		NWorldTimeLight WorldTime;
	};

	struct NLight
	{
		EntityLightMode Mode;
		NLightSettings Settings;
	};

	struct NRenderFlags
	{
		mu_boolean Visible : 1 = false;
		mu_boolean LightEnable : 1 = false; // true : calculate light using normals, false : apply body light directly
	};

	struct NRenderState
	{
		NRenderFlags Flags;
		glm::vec4 BodyLight = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	struct NSkeleton
	{
		mu_uint32 SkeletonOffset = NInvalidUInt32;
		NSkeletonInstance Instance;
	};

	using NBoundingBox = NBoundingBox;

	struct NPosition // Rename
	{
		glm::vec3 Position;
		glm::vec3 Angle;
		mu_float Scale = 1.0f;
	};

	struct NAnimation
	{
		mu_uint16 CurrentAction = 0u;
		mu_uint16 PriorAction = 0u;
		mu_float CurrentFrame = 0.0f;
		mu_float PriorFrame = 0.0f;
	};

	enum class PartType
	{
		CompleteBody,
		Head,
		Helm,
		Armor,
		Pants,
		Gloves,
		Boots,
		ItemLeft,
		ItemRight,
		Wings,
		Helper,
	};

	struct NRenderPart
	{
		PartType Type;
		NModel *Model;
		mu_uint32 Bone;
		NAnimation Animation;
	};

	struct NAttachment
	{
		NModel *Base;
		std::vector<NRenderPart> Parts;
	};
};

#endif