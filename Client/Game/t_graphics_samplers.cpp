#include "stdafx.h"
#include "t_graphics_samplers.h"
#include "mu_graphics.h"

constexpr mu_uint32 TextureFilterBits = ComputeBitsNeeded(Diligent::FILTER_TYPE_NUM_FILTERS);
constexpr mu_uint32 TextureAddressModeBits = ComputeBitsNeeded(Diligent::TEXTURE_ADDRESS_NUM_MODES);
constexpr mu_uint32 SamplerFlagsBits = ComputeBitsNeeded(Diligent::SAMPLER_FLAG_LAST + 1);
constexpr mu_uint32 SamplerStateTotalBits = (
	TextureFilterBits * 3 +
	TextureAddressModeBits * 3 +
	SamplerFlagsBits
);

struct NSamplerStateInfo
{
	Diligent::FILTER_TYPE MinFilter;
	Diligent::FILTER_TYPE MagFilter;
	Diligent::FILTER_TYPE MipFilter;
	Diligent::TEXTURE_ADDRESS_MODE AddressU;
	Diligent::TEXTURE_ADDRESS_MODE AddressV;
	Diligent::TEXTURE_ADDRESS_MODE AddressW;
	Diligent::SAMPLER_FLAGS Flags;
};

typedef mu_uint64 NSamplerHash;
union NSamplerState
{
	NSamplerHash Hash;
	NSamplerStateInfo Info;
};

std::map<NSamplerHash, Diligent::RefCntAutoPtr<Diligent::ISampler>> Samplers;

Diligent::ISampler *CreateTextureSampler(const NSamplerState &state, const Diligent::SamplerDesc &samplerDesc)
{
	const auto device = MUGraphics::GetDevice();
	Diligent::RefCntAutoPtr<Diligent::ISampler> sampler;
	device->CreateSampler(samplerDesc, &sampler);
	if (sampler == nullptr) return nullptr;
	Samplers.insert(std::make_pair(state.Hash, sampler));
	return sampler.RawPtr();
}

Diligent::ISampler *GetTextureSampler(const Diligent::SamplerDesc &samplerDesc)
{
	NSamplerState state;
	state.Info.MinFilter = samplerDesc.MinFilter;
	state.Info.MagFilter = samplerDesc.MagFilter;
	state.Info.MipFilter = samplerDesc.MipFilter;
	state.Info.AddressU = samplerDesc.AddressU;
	state.Info.AddressV = samplerDesc.AddressV;
	state.Info.AddressW = samplerDesc.AddressW;
	state.Info.Flags = samplerDesc.Flags;

	auto iter = Samplers.find(state.Hash);
	if (iter == Samplers.end()) return CreateTextureSampler(state, samplerDesc);
	return iter->second.RawPtr();
}