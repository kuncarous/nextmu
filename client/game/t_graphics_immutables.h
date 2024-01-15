#ifndef __T_GRAPHICS_IMMUTABLES_H__
#define __T_GRAPHICS_IMMUTABLES_H__

#pragma once

enum class NImmutableSampler : mu_uint32
{
	Point_MinMagMip_Clamp_UVW,
	Max,
};
constexpr mu_uint32 MaxImmutableSamplers = static_cast<mu_uint32>(NImmutableSampler::Max);

const Diligent::SamplerDesc GetImmutableSampler(const NImmutableSampler sampler);

#endif