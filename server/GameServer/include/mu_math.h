#ifndef __MU_MATH_H__
#define __MU_MATH_H__

#pragma once

/*
	Instead of use a Matrix (4x4) we will use a simple Position, Scale + Quaternion Rotation structure.
	This might impact a little the CPU however will benefit a lot the GPU and will provide us the chance
	to do Skeleton Skinning in the Vertex Shader.
	THIS IS NOT DUAL QUATERNION. Don't confuse it.

	WARNING!
	Don't move these variables from their current location, it is being used to update the skeleton texture.
*/
struct NCompressedMatrix
{
	glm::quat Rotation;
	glm::vec3 Position;
	mu_float Scale;

	void Set(
		const glm::vec3 angle,
		const glm::vec3 position = glm::vec3(),
		const mu_float scale = 1.0f
	)
	{
		Rotation = glm::quat(glm::radians(angle));
		Position = position;
		Scale = scale;
	}
};

NEXTMU_INLINE const glm::quat AngleToQuaternion(const glm::vec3 v)
{
	return glm::quat(glm::radians(v));
}

NEXTMU_INLINE const glm::vec3 RotateByAngle(const glm::vec3 v, const glm::vec3 angle)
{
	return AngleToQuaternion(angle) * v;
}

NEXTMU_INLINE const glm::vec3 Transform(const glm::vec3 v, const NCompressedMatrix &matrix)
{
	return matrix.Rotation * (v * matrix.Scale) + matrix.Position;
}

NEXTMU_INLINE const glm::vec3 TransformNormal(const glm::vec3 v, const NCompressedMatrix &matrix)
{
	return matrix.Rotation * (v * matrix.Scale);
}

NEXTMU_INLINE const glm::vec3 MovePosition(
	const glm::vec3 &position,
	const glm::vec3 &angle,
	const glm::vec3 &velocity
)
{
	const glm::quat rotation = glm::quat(glm::radians(angle));
	return position + rotation * velocity;
}

#endif
