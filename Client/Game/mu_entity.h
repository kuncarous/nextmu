#ifndef __MU_ENTITY_H__
#define __MU_ENTITY_H__

#pragma once

#include "mu_entity_light.h"
#include "res_render.h"
#include "nav_path.h"

class NModel;
class NSkeletonInstance;
struct NRender;

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

	struct NVisible
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
		std::array<mu_boolean, MAX_CASCADES> ShadowVisible = {};
		glm::vec4 BodyLight = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	struct NSkeleton
	{
		mu_uint32 SkeletonOffset = NInvalidUInt32;
		NSkeletonInstance Instance;
	};

	struct NBoundingBoxes
	{
		NBoundingBox Configured;
		NBoundingBox Calculated;
	};

	struct NPosition // Rename
	{
		glm::vec3 Position;
		glm::vec3 Angle;
		mu_float Scale = 1.0f;
	};

	struct NAction
	{
		mu_uint16 Group = NInvalidUInt16;
		mu_uint16 Index = NInvalidUInt16;
	};

	struct NMovement
	{
		mu_boolean Moving = false;
		NNavPath Path;
	};

	struct NMoveSpeed
	{
		mu_float Walk = 10.0f;
		mu_float Run = 10.0f;
		mu_float Swim = 10.0f;
		mu_float Multiplier = 1.0f;
	};

	struct NAnimation
	{
		mu_uint16 CurrentAction = 0u;
		mu_uint16 PriorAction = 0u;
		mu_float CurrentFrame = 0.0f;
		mu_float PriorFrame = 0.0f;
	};

	enum class PartType : mu_uint32
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
		Max,
	};
	constexpr mu_uint32 MaxPartType = static_cast<mu_uint32>(PartType::Max);

	extern std::array<mu_utf8string, MaxPartType> PartTypeIds;
	NEXTMU_INLINE const mu_utf8string GetPartTypeId(const PartType type)
	{
		return PartTypeIds[static_cast<mu_uint32>(type)];
	}

	struct NRenderLink
	{
		const NRender *Render = nullptr;
		NRenderAnimation RenderAnimation;
		NAnimation Animation;
		NSkeletonInstance Skeleton;
		mu_uint32 Bone = NInvalidUInt32;
		mu_uint32 SkeletonOffset = NInvalidUInt32;
	};

	struct NRenderPart
	{
		PartType Type;
		NModel *Model;
		mu_boolean IsLinked;
		NRenderLink Link;
	};

	struct NAttachment
	{
		NGraphicsTexture *Skin = nullptr;
		NModel *Base;
		std::map<PartType, NRenderPart> Parts;
	};
};

#endif