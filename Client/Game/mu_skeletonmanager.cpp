#include "stdafx.h"
#include "mu_skeletonmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"

namespace MUSkeletonManager
{
	Diligent::RefCntAutoPtr<Diligent::ITexture> BonesTexture;
	std::vector<NCompressedMatrix> BonesBuffer;
	mu_atomic_uint32_t BonesCount = 0;

	const mu_boolean Initialize()
	{
		const auto device = MUGraphics::GetDevice();

		Diligent::TextureDesc textureDesc;
		textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		textureDesc.Width = BonesTextureWidth;
		textureDesc.Height = BonesTextureHeight;
		textureDesc.Format = Diligent::TEX_FORMAT_RGBA32_FLOAT;
		textureDesc.Usage = Diligent::USAGE_DEFAULT;
		textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

		Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
		device->CreateTexture(textureDesc, nullptr, &texture);
		if (texture == nullptr)
		{
			return false;
		}

		BonesTexture = texture;
		BonesBuffer.resize(MaxBonesCount);

		return true;
	}

	void Destroy()
	{
		BonesTexture.Release();
	}

	Diligent::ITexture *GetTexture()
	{
		return BonesTexture.RawPtr();
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

		const auto immediateContext = MURenderState::GetImmediateContext();
		immediateContext->UpdateTexture(
			BonesTexture,
			0, 0,
			Diligent::Box(0, width, 0, height),
			Diligent::TextureSubResData(BonesBuffer.data(), sizeof(glm::vec4) * width),
			Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
		);
	}

	const mu_uint32 UploadBones(const NCompressedMatrix *bones, const mu_uint32 bonesCount)
	{
		const mu_uint32 index = BonesCount.fetch_add(bonesCount);
		mu_assert(index + bonesCount <= MaxBonesCount);
		mu_memcpy(&BonesBuffer[index], bones, sizeof(NCompressedMatrix) * bonesCount);
		return index;
	}
};