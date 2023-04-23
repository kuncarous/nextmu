#ifndef __MU_GRAPHICS_H__
#define __MU_GRAPHICS_H__

#pragma once

#include "t_graphics.h"

namespace MUGraphics
{
	const mu_boolean Initialize();
	void Destroy();

	Diligent::IRenderDevice *GetDevice();
	Diligent::ISwapChain *GetSwapChain();
	Diligent::IDeviceContext *GetImmediateContext();
	NRenderManager *GetRenderManager();

	const char *GetShaderFolder();
	const char *GetShaderExtension();
};

#endif