#include "mu_precompiled.h"
#include "t_graphics_immutables.h"

std::array<Diligent::SamplerDesc, MaxImmutableSamplers> ImmutableSamplers = {
	// Point
	Diligent::SamplerDesc(
		Diligent::FILTER_TYPE_POINT,
		Diligent::FILTER_TYPE_POINT,
		Diligent::FILTER_TYPE_POINT,
		Diligent::TEXTURE_ADDRESS_CLAMP,
		Diligent::TEXTURE_ADDRESS_CLAMP,
		Diligent::TEXTURE_ADDRESS_CLAMP
	),
};

const Diligent::SamplerDesc GetImmutableSampler(const NImmutableSampler sampler)
{
	return ImmutableSamplers[static_cast<mu_uint32>(sampler)];
}