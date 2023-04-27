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

NEXTMU_INLINE Diligent::float4x4 Float4x4FromGLM(glm::mat4 value)
{
	return Diligent::float4x4(
		value[0][0], value[0][1], value[0][2], value[0][3],
		value[1][0], value[1][1], value[1][2], value[1][3],
		value[2][0], value[2][1], value[2][2], value[2][3],
		value[3][0], value[3][1], value[3][2], value[3][3]
	);
}

NEXTMU_INLINE Diligent::float4x4 Float4x4FromCGLM(cglm::mat4 value)
{
	return Diligent::float4x4(
		value[0][0], value[0][1], value[0][2], value[0][3],
		value[1][0], value[1][1], value[1][2], value[1][3],
		value[2][0], value[2][1], value[2][2], value[2][3],
		value[3][0], value[3][1], value[3][2], value[3][3]
	);
}

NEXTMU_INLINE glm::mat4 GLMFromFloat4x4(Diligent::float4x4 value)
{
	return glm::mat4(
		value[0][0], value[0][1], value[0][2], value[0][3],
		value[1][0], value[1][1], value[1][2], value[1][3],
		value[2][0], value[2][1], value[2][2], value[2][3],
		value[3][0], value[3][1], value[3][2], value[3][3]
	);
}

#endif