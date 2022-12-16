#ifndef __MU_TEXTURE_H__
#define __MU_TEXTURE_H__

#pragma once

class NTexture
{
public:
	NTexture(
		const bgfx::TextureHandle texture,
		const mu_uint16 width,
		const mu_uint16 height,
		const mu_boolean alpha
	);
	~NTexture();

public:
	const mu_boolean IsValid() const
	{
		return bgfx::isValid(Texture);
	}

	const mu_boolean HasAlpha() const
	{
		return Alpha;
	}

	const bgfx::TextureHandle GetTexture() const
	{
		return Texture;
	}

private:
	bgfx::TextureHandle Texture = BGFX_INVALID_HANDLE;
	mu_uint16 Width;
	mu_uint16 Height;
	mu_boolean Alpha;
};

#endif