#include "mu_precompiled.h"
#include "t_graphics_texture.h"

NGraphicsTexture::NGraphicsTexture(
	const mu_uint32 id,
	Diligent::RefCntAutoPtr<Diligent::ITexture> texture,
	const mu_uint16 width,
	const mu_uint16 height,
	const mu_boolean alpha
) :
	NGraphicsResource(NGraphicsResourceType::Texture),
	Id(id),
	Texture(texture),
	Width(width),
	Height(height),
	Alpha(alpha)
{

}