#include "mu_precompiled.h"
#include "t_graphics_samplers.h"
#include "mu_graphics.h"

constexpr mu_uint32 TextureFilterBits = ComputeBitsNeeded(Diligent::FILTER_TYPE_NUM_FILTERS);
constexpr mu_uint32 TextureAddressModeBits = ComputeBitsNeeded(Diligent::TEXTURE_ADDRESS_NUM_MODES);
constexpr mu_uint32 SamplerFlagsBits = ComputeBitsNeeded(Diligent::SAMPLER_FLAG_LAST + 1);
constexpr mu_uint32 BoolBits = 1;
constexpr mu_uint32 ComparisonBits = ComputeBitsNeeded(Diligent::COMPARISON_FUNC_NUM_FUNCTIONS);
constexpr mu_uint32 SamplerStateTotalBits = (
	TextureFilterBits * 3 +
	TextureAddressModeBits * 3 +
	SamplerFlagsBits +
	BoolBits +
	ComparisonBits
);

typedef mu_uint32 NSamplerHash;
struct NSamplerState
{
	Diligent::FILTER_TYPE MinFilter;
	Diligent::FILTER_TYPE MagFilter;
	Diligent::FILTER_TYPE MipFilter;
	Diligent::TEXTURE_ADDRESS_MODE AddressU;
	Diligent::TEXTURE_ADDRESS_MODE AddressV;
	Diligent::TEXTURE_ADDRESS_MODE AddressW;
	Diligent::SAMPLER_FLAGS Flags;
	Diligent::Bool UnnormalizedCoords;
	Diligent::COMPARISON_FUNCTION ComparisonFunc;

	const NSamplerHash GetHash() const
	{
		constexpr NSamplerHash MinFilterShift = 0;
		constexpr NSamplerHash MagFilterShift = MinFilterShift + TextureFilterBits;
		constexpr NSamplerHash MipFilterShift = MagFilterShift + TextureFilterBits;
		constexpr NSamplerHash AddressUShift = MipFilterShift + TextureFilterBits;
		constexpr NSamplerHash AddressVShift = AddressUShift + TextureAddressModeBits;
		constexpr NSamplerHash AddressWShift = AddressVShift + TextureAddressModeBits;
		constexpr NSamplerHash FlagsShift = AddressWShift + TextureAddressModeBits;
		constexpr NSamplerHash UnnormalizedCoordsShift = FlagsShift + SamplerFlagsBits;
		constexpr NSamplerHash ComparisonShift = UnnormalizedCoordsShift + BoolBits;

		return (
			(static_cast<NSamplerHash>(MinFilter) << MinFilterShift) |
			(static_cast<NSamplerHash>(MagFilter) << MagFilterShift) |
			(static_cast<NSamplerHash>(MipFilter) << MipFilterShift) |
			(static_cast<NSamplerHash>(AddressU) << AddressUShift) |
			(static_cast<NSamplerHash>(AddressV) << AddressVShift) |
			(static_cast<NSamplerHash>(AddressW) << AddressWShift) |
			(static_cast<NSamplerHash>(Flags) << FlagsShift) |
			(static_cast<NSamplerHash>(UnnormalizedCoords) << UnnormalizedCoordsShift) |
			(static_cast<NSamplerHash>(ComparisonFunc) << ComparisonShift)
		);
	}
};

std::map<NSamplerHash, NSampler> Samplers;

NSampler::NSampler(Diligent::RefCntAutoPtr<Diligent::ISampler> sampler) : Id(GenerateResourceId()), Sampler(sampler)
{}

NSampler *CreateTextureSampler(const NSamplerState &state, const Diligent::SamplerDesc &samplerDesc)
{
	const auto device = MUGraphics::GetDevice();
	Diligent::RefCntAutoPtr<Diligent::ISampler> sampler;
	device->CreateSampler(samplerDesc, &sampler);
	if (sampler == nullptr) return nullptr;

	auto iter = Samplers.insert(std::make_pair(state.GetHash(), NSampler(sampler))).first;
	return &iter->second;
}

NSampler *GetTextureSampler(const Diligent::SamplerDesc &samplerDesc)
{
	NSamplerState state;
	state.MinFilter = samplerDesc.MinFilter;
	state.MagFilter = samplerDesc.MagFilter;
	state.MipFilter = samplerDesc.MipFilter;
	state.AddressU = samplerDesc.AddressU;
	state.AddressV = samplerDesc.AddressV;
	state.AddressW = samplerDesc.AddressW;
	state.Flags = samplerDesc.Flags;
	state.UnnormalizedCoords = samplerDesc.UnnormalizedCoords;
	state.ComparisonFunc = samplerDesc.ComparisonFunc;

	auto iter = Samplers.find(state.GetHash());
	if (iter == Samplers.end()) return CreateTextureSampler(state, samplerDesc);
	return &iter->second;
}