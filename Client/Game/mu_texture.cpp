#include "stdafx.h"
#include "mu_texture.h"
#include "mu_textures.h"

NTexture::NTexture(
	const bgfx::TextureHandle texture,
	const mu_uint16 width,
	const mu_uint16 height,
	const mu_boolean alpha
) :
	Texture(texture),
	Width(width),
	Height(height),
	Alpha(alpha)
{

}

NTexture::~NTexture()
{
	if (bgfx::isValid(Texture))
	{
		bgfx::destroy(Texture);
		Texture = BGFX_INVALID_HANDLE;
	}
}