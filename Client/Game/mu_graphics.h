#ifndef __MU_GRAPHICS_H__
#define __MU_GRAPHICS_H__

#pragma once

namespace MUGraphics
{
	const mu_boolean Initialize();
	void Destroy();

	const char *GetShaderFolder();
	const char *GetShaderExtension();
};

#endif