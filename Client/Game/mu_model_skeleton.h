#ifndef __MU_MODEL_SKELETON_H__
#define __MU_MODEL_SKELETON_H__

#pragma once

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
	mu_boolean Loop = false;
	mu_boolean LockPositions = false;
	mu_float PlaySpeed = 1.0f;
	std::vector<NAnimationKey> Keys; // Per Animation Frame
};

class NBoneInfo
{
public:
	mu_boolean Dummy = true;
	mu_int16 Parent = NInvalidInt16;
};

// Should change the bounding box to use a center and extent instead?, which one would benefit more at future?
constexpr mu_float InvalidBox = 50000.0f;
class NBoneBoundingBox
{
public:
	glm::vec3 Min = glm::vec3(InvalidBox, InvalidBox, InvalidBox);
	glm::vec3 Max = glm::vec3(-InvalidBox, -InvalidBox, -InvalidBox);
};

#endif