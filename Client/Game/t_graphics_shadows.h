#ifndef __T_GRAPHICS_SHADOWS_H__
#define __T_GRAPHICS_SHADOWS_H__

#pragma once

enum class NShadowMode : mu_uint32
{
	PCF = SHADOW_MODE_PCF,
	VSM = SHADOW_MODE_VSM,
	EVSM2 = SHADOW_MODE_EVSM2,
	EVSM4 = SHADOW_MODE_EVSM4,
};
const mu_uint32 MinShadowMode = static_cast<mu_uint32>(NShadowMode::PCF);
const mu_uint32 MaxShadowMode = static_cast<mu_uint32>(NShadowMode::EVSM4);

constexpr Diligent::TEXTURE_FORMAT ShadowMapDepthFormat = Diligent::TEX_FORMAT_D16_UNORM;
constexpr mu_float ShadowMapMinimumValue = 0.3f;

#endif