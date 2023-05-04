#ifndef __MU_MODEL_SKELETON_H__
#define __MU_MODEL_SKELETON_H__

#pragma once

#include "mu_math_aabb.h"
#include "t_model_enums.h"

class NBone
{
public:
	glm::vec3 Position = glm::vec3();
	glm::quat Rotation = glm::quat();
};

class NAnimationKey
{
public:
	std::vector<NBone> Bones; // Per Bone
};

class NAnimation
{
public:
	mu_utf8string Id;
	mu_boolean Loop = false;
	mu_boolean LockPositions = false;
	NAnimationModifierType Modifier = NAnimationModifierType::None;
	mu_float PlaySpeed = 1.0f;
	std::vector<NAnimationKey> Keys; // Per Animation Frame
};

class NBoneInfo
{
public:
	mu_boolean Dummy = true;
	mu_int16 Parent = NInvalidInt16;
};

class NBoundingBoxWithValidation : public NBoundingBoxWithDefault
{
public:
	mu_boolean Valid = false;
};

class NModelBoundingBoxes
{
public:
	mu_boolean Valid = false;
	NBoundingBox Global;
	std::vector<NBoundingBox> PerAnimation;
};

#endif