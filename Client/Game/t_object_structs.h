#ifndef __T_OBJECT_STRUCTS_H__
#define __T_OBJECT_STRUCTS_H__

#pragma once

#include "mu_entity_light.h"

namespace TObject
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
		mu_uint32 FadingGroup;
		LightSettings Light;
		glm::vec3 Position;
		glm::vec3 Angle;
		mu_float Scale;
		glm::vec3 BBoxMin;
		glm::vec3 BBoxMax;
	};
};

#endif