#ifndef __MU_SKELETONMANAGER_H__
#define __MU_SKELETONMANAGER_H__

#pragma once

struct NCompressedBone;

namespace MUSkeletonManager
{
	const mu_boolean Initialize();
	void Destroy();

	const bgfx::TextureHandle GetTexture();
	const bgfx::UniformHandle GetSampler();

	void Reset();
	void Update();

	const mu_uint32 UploadBones(const NCompressedBone *bones, const mu_uint32 bonesCount);
}

#endif