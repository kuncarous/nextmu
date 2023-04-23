#ifndef __MU_RESOURCES_H__
#define __MU_RESOURCES_H__

#pragma once

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
	Diligent::RefCntAutoPtr<Diligent::ITexture> Handle;
};

#endif