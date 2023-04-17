#include "stdafx.h"
#include "mu_skeletoninstance.h"
#include "mu_model.h"
#include "mu_skeletonmanager.h"

void NSkeletonInstance::Animate(
	const NModel *Model,
	AnimationFrameInfo Current,
	AnimationFrameInfo Prior,
	const glm::vec3 HeadAngle
)
{
	const mu_uint32 numAnimations = static_cast<mu_uint32>(Model->Animations.size());
	const mu_uint32 numBones = static_cast<mu_uint32>(Model->BoneInfo.size());

	mu_assert(numAnimations > 0);
	mu_assert(numBones > 0);

	if (Bones.size() < numBones) Bones.resize(numBones);

	if (Current.Action >= numAnimations) Current.Action = 0;
	if (Current.Frame < 0.0f) Current.Frame = 0.0f;
	if (Prior.Action >= numAnimations) Prior.Action = 0;
	if (Prior.Frame < 0.0f) Prior.Frame = 0.0f;

	mu_uint32 current = static_cast<mu_uint32>(Current.Frame);
	mu_uint32 prior = static_cast<mu_uint32>(Prior.Frame);
	mu_uint32 currentNext = current + 1u;
	mu_uint32 priorNext = prior + 1u;

	const auto &currentAnimation = Model->Animations[Current.Action];
	const auto &priorAnimation = Model->Animations[Prior.Action];
	const auto &currentFramesCount = static_cast<mu_uint32>(currentAnimation.Keys.size());
	const auto &priorFramesCount = static_cast<mu_uint32>(priorAnimation.Keys.size());

	if (currentAnimation.Loop)
	{
		if (current >= currentFramesCount) current = currentFramesCount - 1;
		if (currentNext >= currentFramesCount) currentNext = currentFramesCount - 1;
	}
	else
	{
		current = current % currentFramesCount;
		currentNext = currentNext % currentFramesCount;
	}

	if (priorAnimation.Loop)
	{
		if (prior >= priorFramesCount) prior = priorFramesCount - 1;
		if (priorNext >= priorFramesCount) priorNext = priorFramesCount - 1;
	}
	else
	{
		prior = prior % priorFramesCount;
		priorNext = priorNext % priorFramesCount;
	}

	/*
		Why I processed 4 frames instead of only 2 frames?
		To provide a correct animation blending in high framerates.
	*/
	const auto &currentFrame1 = currentAnimation.Keys[current];
	const auto &currentFrame2 = currentAnimation.Keys[currentNext];
	const auto &priorFrame1 = priorAnimation.Keys[prior];
	const auto &priorFrame2 = priorAnimation.Keys[priorNext];
	const auto boneHead = static_cast<mu_uint32>(Model->BoneHead);

	const mu_float s1 = Current.Frame - glm::floor(Current.Frame);
	const mu_float s2 = 1.0f - s1;
	const mu_float ps1 = Prior.Frame - glm::floor(Prior.Frame);
	const mu_float ps2 = 1.0f - ps1;

	const mu_boolean lockPosition = priorAnimation.LockPositions || currentAnimation.LockPositions;
	const glm::quat headRotation(glm::vec3(glm::radians(-HeadAngle[0]), 0.0f, glm::radians(-HeadAngle[1])));
	for (mu_uint32 b = 0; b < numBones; ++b)
	{
		auto &info = Model->BoneInfo[b];
		if (info.Dummy) continue;

		const auto &currentBone1 = currentFrame1.Bones[b];
		const auto &currentBone2 = currentFrame2.Bones[b];
		const auto &priorBone1 = priorFrame1.Bones[b];
		const auto &priorBone2 = priorFrame2.Bones[b];

		const auto &currentRotation1 = currentBone1.Rotation;
		const auto &currentRotation2 = currentBone2.Rotation;
		const auto &priorRotation1 = priorBone1.Rotation;
		const auto &priorRotation2 = priorBone2.Rotation;

		glm::quat currentRotation = glm::slerp(currentRotation1, currentRotation2, s1);
		glm::quat priorRotation = glm::slerp(priorRotation1, priorRotation2, ps1);

		if (b == boneHead)
		{
			currentRotation *= headRotation;
			priorRotation *= headRotation;
		}

		const auto &currentPosition1 = currentBone1.Position;
		const auto &currentPosition2 = currentBone2.Position;
		const auto &priorPosition1 = priorBone1.Position;
		const auto &priorPosition2 = priorBone2.Position;

		const auto currentPosition = currentPosition1 * s2 + currentPosition2 * s1;
		const auto priorPosition = priorPosition1 * ps2 + priorPosition2 * ps1;

		auto &outBone = Bones[b];
		outBone.Rotation = glm::slerp(priorRotation, currentRotation, s1);
		outBone.Scale = 1.0f;

		if (b == 0 && lockPosition)
		{
			outBone.Position[0] = currentBone1.Position[0];
			outBone.Position[1] = currentBone1.Position[1];
			outBone.Position[2] = priorPosition[2] * s2 + currentPosition[2] * s1 + Model->BodyHeight;
		}
		else
		{
			outBone.Position = priorPosition * s2 + currentPosition * s1;
		}

		mu_assert(info.Parent == NInvalidInt16 || (info.Parent >= 0 && info.Parent < static_cast<mu_int16>(numBones)));
		MixBones(
			info.Parent == NInvalidInt16
			? Parent
			: Bones[info.Parent],
			outBone
		);
	}

	BonesCount = numBones;
}

const mu_uint32 NSkeletonInstance::Upload()
{
	if (BonesCount == 0) return NInvalidUInt32;
	return MUSkeletonManager::UploadBones(Bones.data(), BonesCount);
}