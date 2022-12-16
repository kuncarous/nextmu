#ifndef __MU_ENVIRONMENT_STRUCTS_H__
#define __MU_ENVIRONMENT_STRUCTS_H__

#pragma once

class NModel;
class NSkeletonInstance;

namespace NEntity
{
	struct Attachment
	{
		const NModel *Model;
	};

	struct Skeleton
	{
		mu_uint32 SkeletonOffset;
		NSkeletonInstance Instance;
	};

	struct Position // Rename
	{
		glm::vec3 Position;
		glm::vec3 Angle;
		mu_float Scale;
	};

	struct Animation
	{
		mu_uint16 CurrentAction = 0u;
		mu_uint16 PriorAction = 0u;
		mu_float CurrentFrame = 0.0f;
		mu_float PriorFrame = 0.0f;
	};
};

#endif