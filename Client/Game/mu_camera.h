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

struct NCameraDefault
{
	glm::vec3 Eye, Target, Angle, Up;
	NCameraDistance Distance;
};

class NCamera
{
public:
	void Update();
	void GenerateFrustum(glm::mat4 view, glm::mat4 projection);

	glm::mat4 GetView();

	void SetMode(NCameraMode mode);
	void SetEye(glm::vec3 eye);
	void SetTarget(glm::vec3 target);
	void SetAngle(glm::vec3 angle);
	void SetUp(glm::vec3 up);
	void SetDistance(const mu_float distance);
	void SetMinDistance(const mu_float minDistance);
	void SetMaxDistance(const mu_float maxDistance);

	const NCameraMode GetMode() const;
	glm::vec3 GetEye() const;
	glm::vec3 GetTarget() const;
	glm::vec3 GetAngle() const;
	glm::vec3 GetUp() const;
	const mu_float GetDistance() const;
	const mu_float GetMinDistance() const;
	const mu_float GetMaxDistance() const;

public:
	const Diligent::ViewFrustumExt *GetFrustum() const
	{
		return &Frustum;
	}

	const Diligent::BoundBox GetFrustumBBox() const
	{
		return FrustumBBox;
	}

	void SaveAsDefault()
	{
		Default.Eye = Eye;
		Default.Target = Target;
		Default.Angle = Angle;
		Default.Up = Up;
		Default.Distance = Distance;
	}

	void RestoreDefault()
	{
		if (Mode == NCameraMode::Directional)
		{
			Eye = Default.Eye;
		}

		Angle = Default.Angle;
		Up = Default.Up;
		Distance = Default.Distance;
	}

private:
	Diligent::ViewFrustumExt Frustum;
	Diligent::BoundBox FrustumBBox;
	NCameraMode Mode = NCameraMode::Directional;
	glm::vec3 Eye;
	glm::vec3 Target;
	glm::vec3 Angle;
	glm::vec3 Up;
	NCameraDistance Distance;
	NCameraDefault Default;

	glm::ivec2 BackupMousePosition = glm::ivec2();
	glm::vec2 MouseDelta = glm::vec2();

	mu_boolean Dragging = false;
};

#endif