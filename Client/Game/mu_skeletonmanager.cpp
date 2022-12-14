#include "stdafx.h"
#include "mu_skeletonmanager.h"
#include "mu_skeletoninstance.h"

namespace MUSkeletonManager
{
	/*
		2048x512 can handle around ~2600 characters with 200 bones per character.
		This texture will consume 16MB of video memory, not much but enough.
	*/
	constexpr mu_uint32 BonesTextureWidth = 2048;
	constexpr mu_uint32 BonesTextureHeight = 512;
	constexpr mu_uint32 MaxBonesCount = (BonesTextureWidth * BonesTextureHeight) / 2u;

	bgfx::TextureHandle BonesTexture = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle BonesSampler = BGFX_INVALID_HANDLE;
	std::vector<NCompressedBone> BonesBuffer;
	mu_atomic_uint32_t BonesCount = 0;

	const mu_boolean Initialize()
	{
		BonesTexture = bgfx::createTexture2D(BonesTextureWidth, BonesTextureHeight, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT);
		if (bgfx::isValid(BonesTexture) == false)
		{
			return false;
		}

		BonesSampler = bgfx::createUniform("s_skeletonTexture", bgfx::UniformType::Sampler);
		if (bgfx::isValid(BonesSampler) == false)
		{
			return false;
		}

		BonesBuffer.resize(MaxBonesCount);

		return true;
	}

	void Destroy()
	{
		if (bgfx::isValid(BonesTexture))
		{
			bgfx::destroy(BonesTexture);
			BonesTexture = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(BonesSampler))
		{
			bgfx::destroy(BonesSampler);
			BonesSampler = BGFX_INVALID_HANDLE;
		}
	}

	const bgfx::TextureHandle GetTexture()
	{
		return BonesTexture;
	}

	const bgfx::UniformHandle GetSampler()
	{
		return BonesSampler;
	}

	void Reset()
	{
		BonesCount.store(0u, std::memory_order_relaxed);
	}

	void Update()
	{
		const mu_uint32 bonesCount = BonesCount.load(std::memory_order_relaxed);
		if (bonesCount <= 0) return;

		const mu_uint32 pixelsCount = bonesCount * 2;
		const mu_uint32 extraHeight = glm::step(1u, pixelsCount % BonesTextureWidth);
		const mu_uint32 width = glm::clamp(pixelsCount, 1u, BonesTextureWidth);
		const mu_uint32 height = glm::clamp((pixelsCount / BonesTextureWidth) + extraHeight, 1u, BonesTextureHeight);

		const bgfx::Memory *mem = bgfx::makeRef(BonesBuffer.data(), sizeof(glm::vec4) * width * height);
		bgfx::updateTexture2D(BonesTexture, 0, 0, 0, 0, width, height, mem);
	}

	const mu_uint32 UploadBones(const NCompressedBone *bones, const mu_uint32 bonesCount)
	{
		const mu_uint32 index = BonesCount.fetch_add(bonesCount);
		mu_assert(index + bonesCount <= MaxBonesCount);
		mu_memcpy(&BonesBuffer[index], bones, sizeof(NCompressedBone) * bonesCount);
		return index;
	}
};