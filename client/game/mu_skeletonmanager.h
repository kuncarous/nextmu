#ifndef __MU_SKELETONMANAGER_H__
#define __MU_SKELETONMANAGER_H__

#pragma once

struct NCompressedMatrix;

namespace MUSkeletonManager
{
	/*
		2048x512 can handle around ~2600 characters with 200 bones per character.
		This texture will consume 16MB of video memory, not much but enough.
	*/
	constexpr mu_uint32 BonesTextureWidth = 2048;
	constexpr mu_uint32 BonesTextureHeight = 512;
	constexpr mu_uint32 MaxBonesCount = (BonesTextureWidth * BonesTextureHeight) / 2u;

	const mu_boolean Initialize();
	void Destroy();

	Diligent::ITexture *GetTexture();

	void Reset();
	void Update();

	const mu_uint32 UploadBones(const NCompressedMatrix *bones, const mu_uint32 bonesCount);
}

#endif