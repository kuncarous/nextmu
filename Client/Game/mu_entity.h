#ifndef __MU_ENTITY_H__
#define __MU_ENTITY_H__

#pragma once

#include "mu_entity_light.h"
#include "res_render.h"
#include "nav_path.h"
#include "t_model_enums.h"
#include "t_character_structs.h"

class NModel;
class NSkeletonInstance;
struct NRender;
struct NAnimationsRoot;
struct NCharacterConfiguration;

enum class NAnimationType : mu_uint16
{
	Appear,
	Stop,
	Walk,
	Run,
	Attack,
	Shock,
	Die,
	Max,
};
constexpr mu_uint32 AnimationTypeMax = static_cast<mu_uint32>(NAnimationType::Max);
extern std::array<const mu_utf8string, AnimationTypeMax> AnimationTypeStrings;

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

	struct NCharacterInfo
	{
		NCharacterInfo() : Type(CharacterType::Character), MonsterType(0) {}

		CharacterType Type;
		union
		{
			mu_uint32 MonsterType;
			NCharacterType CharacterType;
		};
	};

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
		mu_boolean Visible = false;
		mu_boolean LightEnable = false; // true : calculate light using normals, false : apply body light directly
		mu_boolean ShouldFade = false;
	};

	struct NRenderFading
	{
		mu_boolean Enabled = false;
		mu_float Current = 1.0f;
		mu_float Target = 0.2f;
	};

	struct NRenderState
	{
		NRenderFlags Flags;
		std::array<mu_boolean, MAX_CASCADES> ShadowVisible = {};
		glm::vec4 BodyLight = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		NRenderFading Fading;
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
		glm::vec3 HeadAngle = glm::vec3(0.0f, 0.0f, 0.0f);
		mu_float Scale = 1.0f;
	};

	struct NMovement
	{
		mu_boolean Moving = false;
		NNavPath Path;
	};

	struct NMoveSpeed
	{
		mu_float Walk = 12.0f;
		mu_float Run = 15.0f;
		mu_float Swim = 15.0f;
	};

	struct NAnimation
	{
		NAnimationModifierType ModifierType = NAnimationModifierType::None;
		mu_uint16 CurrentAction = 0u;
		mu_uint16 PriorAction = 0u;
		mu_float CurrentFrame = 0.0f;
		mu_float PriorFrame = 0.0f;
	};

	struct NModifiers
	{
		struct
		{
			mu_float MoveSpeed = 1.0f;
			mu_float AttackSpeed = 1.0f;
		} Current;

		struct
		{
			mu_float MoveSpeed = 1.0f;
			mu_float AttackSpeed = 1.0f;
		} Normalized;
	};

	struct NAnimationsMapping
	{
		const NAnimationsRoot *Root = nullptr;
		std::map<NAnimationType, mu_uint32> Normal;
		std::map<NAnimationType, mu_uint32> Safezone;
	};

	struct NAction
	{
		NAnimationType Type = NAnimationType::Stop;
	};

	struct NLinkAnimation
	{
		mu_uint32 Bone = NInvalidUInt32;
		glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Angle = glm::vec3(0.0f, 0.0f, 0.0f);
		mu_float Scale = 1.0f;
	};

	struct NRenderLink
	{
		const NRender *Render = nullptr;
		NLinkAnimation RenderAnimation;
		NAnimation Animation;
		NSkeletonInstance Skeleton;
		mu_uint32 SkeletonOffset = NInvalidUInt32;
	};

	struct NRenderPart
	{
		NPartType Type;
		NModel *Model;
		mu_boolean IsLinked;
		NRenderLink Link;
	};

	struct NAttachment
	{
		const NCharacterConfiguration *Character;
		NModel *Base;
		std::map<NPartType, NRenderPart> Parts;
	};
};

#endif