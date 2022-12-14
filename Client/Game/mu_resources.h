#ifndef __MU_RESOURCES_H__
#define __MU_RESOURCES_H__

#pragma once

#include <bgfx/bgfx.h>

enum class NResourceType
{
	Texture,
	Uniform,
	VertexBuffer,
	IndexBuffer,
	Shader,
	Program,
};

class NBaseResource
{
public:
	NResourceType Type;
};

class NTextureResource
{
public:
	bgfx::TextureHandle Handle;
};

#endif