#ifndef __MU_TEXTURE_H__
#define __MU_TEXTURE_H__

#pragma once

class NTexture
{
public:
	NTexture(
		Diligent::RefCntAutoPtr<Diligent::ITexture> texture,
		const mu_uint16 width,
		const mu_uint16 height,
		const mu_boolean alpha
	);
	~NTexture();

public:
	const mu_boolean IsValid() const
	{
		return Texture != nullptr;
	}

	const mu_boolean HasAlpha() const
	{
		return Alpha;
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
	Diligent::RefCntAutoPtr<Diligent::ITexture> Texture;
	mu_uint16 Width;
	mu_uint16 Height;
	mu_boolean Alpha;
};

#endif