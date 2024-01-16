#ifndef __MU_SKELETONINSTANCE_H__
#define __MU_SKELETONINSTANCE_H__

#pragma once

class NModel;

NEXTMU_INLINE void MixBones(
	const NCompressedMatrix &parent,
	NCompressedMatrix &out
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
	void Animate(
		const NModel *Model,
		AnimationFrameInfo Current,
		AnimationFrameInfo Prior,
		const glm::vec3 HeadAngle
	);

	const mu_uint32 Upload();

public:
	void SetParent(
		const glm::vec3 Angle,
		const glm::vec3 Position = glm::vec3(),
		const mu_float Scale = 1.0f
	)
	{
		Parent.Set(Angle, Position, Scale);
	}

	void SetParent(NCompressedMatrix matrix)
	{
		Parent = matrix;
	}

	const NCompressedMatrix &GetParent() const
	{
		return Parent;
	}

	const NCompressedMatrix &GetBone(const mu_uint32 bone) const
	{
		return Bones[bone];
	}

private:
	NCompressedMatrix Parent;
	std::vector<NCompressedMatrix> Bones;
	mu_uint32 BonesCount;
};

#endif