#ifndef __T_GRAPHICS_TEXTURE_H__
#define __T_GRAPHICS_TEXTURE_H__

#pragma once

#include "t_graphics_resources.h"

class NGraphicsTexture : public NGraphicsResource
{
public:
	NGraphicsTexture(
		const mu_uint32 id,
		Diligent::RefCntAutoPtr<Diligent::ITexture> texture,
		const mu_uint16 width,
		const mu_uint16 height,
		const mu_boolean alpha
	);

public:
	const mu_boolean IsValid() const
	{
		return Texture != nullptr;
	}

	const mu_boolean HasAlpha() const
	{
		return Alpha;
	}

	const mu_uint32 GetId() const
	{
		return Id;
	}

	Diligent::ITexture *GetTexture()
	{
		return Texture.RawPtr();
	}

	const mu_uint16 GetWidth() const
	{
		return Width;
	}

	const mu_uint16 GetHeight() const
	{
		return Height;
	}

private:
	mu_uint32 Id;
	Diligent::RefCntAutoPtr<Diligent::ITexture> Texture;
	mu_uint16 Width;
	mu_uint16 Height;
	mu_boolean Alpha;
};

#endif