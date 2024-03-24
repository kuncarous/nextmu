#ifndef __MU_CAMERA_H__
#define __MU_CAMERA_H__

#pragma once

enum class NCameraMode
{
	DirectionalByAngle, // Target depends on Eye and Direction (except that direction is calculated directly using Angle with InverseUP)
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
	glm::vec3 Eye, Target, Angle;
	NCameraDistance Distance;
};

class NCamera
{
public:
	void Update();
	void GenerateFrustum(glm::mat4 view, glm::mat4 projection, const mu_float nearZ, const mu_float farZ);

	glm::mat4 GetView() const;
	glm::mat4 GetShadowView() const;

	void SetMode(const NCameraMode mode);
	void SetFOV(const mu_float fov);
	void SetEye(const glm::vec3 eye);
	void SetTarget(const glm::vec3 target);
	void SetAngle(const glm::vec3 angle);
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
		Default.Distance = Distance;
	}

	void RestoreDefault()
	{
		if (Mode == NCameraMode::Directional)
		{
			Eye = Default.Eye;
		}

		Angle = Default.Angle;
		Distance = Default.Distance;
	}

public:
	void SetCanDrag(mu_boolean value)
	{
		CanDrag = value;
	}

private:
	Diligent::ViewFrustumExt Frustum;
	Diligent::BoundBox FrustumBBox;
	NCameraMode Mode = NCameraMode::Directional;
	mu_float FOV = 35.0f;
	glm::vec3 Eye;
	glm::vec3 Target;
	glm::vec3 Angle;
	const glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 InverseUp = -Up;
	NCameraDistance Distance;
	NCameraDefault Default;

	glm::ivec2 BackupMousePosition = glm::ivec2();
	glm::vec2 MouseDelta = glm::vec2();

	mu_boolean CanDrag = true;
	mu_boolean Dragging = false;
};

#endif