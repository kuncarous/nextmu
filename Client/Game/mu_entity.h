#ifndef __MU_ENTITY_H__
#define __MU_ENTITY_H__

#pragma once

#include "mu_entity_light.h"

class NModel;
class NSkeletonInstance;
namespace NEntity
{
	struct Identifier
	{
		const mu_key Key;
	};

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

	struct RenderFlags
	{
		mu_boolean Visible : 1;
		mu_boolean LightEnable : 1; // true : calculate light using normals, false : apply body light directly
	};

	struct RenderState
	{
		RenderFlags Flags;
		glm::vec4 BodyLight;
	};

	struct Skeleton
	{
		mu_uint32 SkeletonOffset;
		NSkeletonInstance Instance;
	};

	template<typename T>
	NEXTMU_INLINE void Swap(T &a, T &b)
	{
		T tmp = a;
		a = b;
		b = tmp;
	}

	class BoundingBox : public NBoundingBox
	{
	public:
		void Order()
		{
			if (Min.x > Max.x) Swap(Min.x, Max.x);
			if (Min.y > Max.y) Swap(Min.y, Max.y);
			if (Min.z > Max.z) Swap(Min.z, Max.z);
		}
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