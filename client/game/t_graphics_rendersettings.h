#ifndef __T_GRAPHICS_RENDERSETTINGS_H__
#define __T_GRAPHICS_RENDERSETTINGS_H__

#pragma once

enum class NRenderMode : mu_uint32
{
	Normal,
	ShadowMap,
};

struct NRenderSettings
{
	mu_uint32 ShadowFrustumsNum = 0u;
	mu_uint32 CurrentShadowMap = 0u;
	const Diligent::ViewFrustumExt *ShadowFrustums = nullptr;
	const Diligent::ViewFrustumExt *Frustum = nullptr;
};

#endif