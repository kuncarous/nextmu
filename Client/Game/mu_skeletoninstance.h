#ifndef __MU_SKELETONINSTANCE_H__
#define __MU_SKELETONINSTANCE_H__

#pragma once

class NModel;

/*
	Instead of use a Matrix (4x4) we will use a simple Position, Scale + Quaternion Rotation structure.
	This might impact a little the CPU however will benefit a lot the GPU and will provide us the chance
	to do Skeleton Skinning in the Vertex Shader.
	THIS IS NOT DUAL QUATERNION. Don't confuse it.

	WARNING!
	Don't move these variables from their current location, it is being used to update the skeleton texture.
*/
struct NCompressedBone
{
	glm::quat Rotation;
	glm::vec3 Position;
	mu_float Scale;
};

NEXTMU_INLINE void MixBones(
	const NCompressedBone &parent,
	NCompressedBone &out
)
{
	out.Rotation = parent.Rotation * out.Rotation;
	out.Position = parent.Rotation * (out.Position * parent.Scale) + parent.Position;
	out.Scale = parent.Scale;
}

struct SkeletonInfo
{
	mu_boolean Parent;
};

struct AnimationFrameInfo
{
	mu_uint16 Action;
	mu_float Frame;
};

class NSkeletonInstance
{
public:
	void SetParent(
		const glm::vec3 Angle,
		const glm::vec3 Position = glm::vec3(),
		const mu_float Scale = 1.0f
	);

	/*
		Return tells if can continue same animation, if false means the animation finished.
	*/
	const mu_boolean PlayAnimation(
		const NModel *Model,
		mu_uint16 &CurrentAction,
		mu_uint16 &PriorAction,
		mu_float &CurrentFrame,
		mu_float &PriorFrame,
		const mu_float PlaySpeed
	);

	void Animate(
		const NModel *Model,
		AnimationFrameInfo Current,
		AnimationFrameInfo Prior,
		const glm::vec3 HeadAngle
	);

	const mu_uint32 Upload();

private:
	NCompressedBone Parent;
	std::vector<NCompressedBone> Bones;
	mu_uint32 BonesCount;
};

#endif