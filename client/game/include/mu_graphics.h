#ifndef __MU_GRAPHICS_H__
#define __MU_GRAPHICS_H__

#pragma once

#include "t_graphics.h"

namespace MUGraphics
{
	const mu_boolean Initialize();
	void Destroy();

	NRenderTargetDesc &GetRenderTargetDesc();
	void SetRenderTargetDesc(const NRenderTargetDesc desc);

	mu_boolean IssRGB();
	Diligent::RENDER_DEVICE_TYPE GetDeviceType();
	Diligent::IRenderDevice *GetDevice();
	Diligent::ISwapChain *GetSwapChain();
	Diligent::IDeviceContext *GetImmediateContext();
	NRenderManager *GetRenderManager();
};

#endif