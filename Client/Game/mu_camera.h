#ifndef __MU_CAMERA_H__
#define __MU_CAMERA_H__

#pragma once

#include "mu_math_frustum.h"

enum class NCameraMode
{
	Directional, // Target depends on Eye and Direction
	Positional, // Eye depends on Target and Direction
};

struct NCameraDistance
{
	mu_float Current = 0.0f;
	mu_float Minimum = 0.0f;
	mu_float Maximum = 0.0f;
};

class NCamera
{
public:
	void Update();
	void GenerateFrustum(cglm::mat4 view, cglm::mat4 projection);

	void GetView(cglm::mat4 out);

	void SetMode(NCameraMode mode);
	void SetEye(glm::vec3 eye);
	void SetTarget(glm::vec3 target);
	void SetAngle(glm::vec3 angle);
	void SetUp(glm::vec3 up);
	void SetDistance(const mu_float distance);
	void SetMinDistance(const mu_float minDistance);
	void SetMaxDistance(const mu_float maxDistance);

	void GetEye(cglm::vec3 out);
	void GetTarget(cglm::vec3 out);
	void GetAngle(cglm::vec3 out);
	void GetUp(cglm::vec3 out);
	const mu_float GetDistance();
	const mu_float GetMinDistance();
	const mu_float GetMaxDistance();

public:
	const NMathFrustum *GetFrustum() const
	{
		return &Frustum;
	}

private:
	NMathFrustum Frustum;
	NCameraMode Mode = NCameraMode::Directional;
	cglm::vec3 Eye = {};
	cglm::vec3 Target = {};
	cglm::vec3 Angle = {};
	cglm::vec3 Up = {};
	NCameraDistance Distance;

	glm::ivec2 BackupMousePosition = glm::ivec2();
	glm::vec2 MouseDelta = glm::vec2();

	mu_boolean Dragging = false;
};

#endif