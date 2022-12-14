#ifndef __MU_CAMERA_H__
#define __MU_CAMERA_H__

#pragma once

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

	const glm::mat4 GetView();

	void SetMode(NCameraMode mode);
	void SetEye(const glm::vec3 eye);
	void SetTarget(const glm::vec3 target);
	void SetAngle(const glm::vec3 angle);
	void SetUp(const glm::vec3 up);
	void SetDistance(const mu_float distance);
	void SetMinDistance(const mu_float minDistance);
	void SetMaxDistance(const mu_float maxDistance);

	const glm::vec3 GetEye();
	const glm::vec3 GetTarget();
	const glm::vec3 GetAngle();
	const glm::vec3 GetUp();
	const mu_float GetDistance();
	const mu_float GetMinDistance();
	const mu_float GetMaxDistance();

private:
	NCameraMode Mode = NCameraMode::Directional;
	glm::vec3 Eye = glm::vec3();
	glm::vec3 Target = glm::vec3();
	glm::vec3 Angle = glm::vec3();
	glm::vec3 Up = glm::vec3();
	NCameraDistance Distance;

	glm::ivec2 BackupMousePosition = glm::ivec2();
	glm::vec2 MouseDelta = glm::vec2();

	mu_boolean Dragging = false;
};

#endif