#include "stdafx.h"
#include "ui_noesisgui_texture.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	DETexture::DETexture(
		mu_uint32 width,
		mu_uint32 height,
		mu_uint32 levels,
		mu_uint32 sampleCount,
		mu_boolean isInverted,
		mu_boolean hasAlphaChannel,
		Diligent::TEXTURE_FORMAT format,
		Diligent::RefCntAutoPtr<Diligent::ITexture> texture,
		mu_boolean transitionRequired
	) :
		ResourceId(GenerateResourceId()),
		Width(width),
		Height(height),
		Levels(levels),
		SampleCount(sampleCount),
		Inverted(isInverted),
		Alpha(hasAlphaChannel),
		Format(format),
		Texture(texture),
		TransitionRequired(transitionRequired)
	{

	}

	DETexture::~DETexture()
	{
		Texture.Release();
	}

	/// Returns the width of the texture, in pixels
	uint32_t DETexture::GetWidth() const
	{
		return Width;
	}

	/// Returns the height of the texture, in pixels
	uint32_t DETexture::GetHeight() const
	{
		return Height;
	}

	uint32_t DETexture::GetSampleCount() const
	{
		return SampleCount;
	}

	/// Returns true if the texture has mipmaps
	bool DETexture::HasMipMaps() const
	{
		return Levels > 1;
	}

	/// Returns true when texture must be vertically inverted when mapped. This is true for surfaces
	/// on platforms (OpenGL) where texture V coordinate is zero at the "bottom of the texture"
	bool DETexture::IsInverted() const
	{
		return Inverted;
	}

	/// Returns true if the texture has an alpha channel that is not completely white. Just a hint
	/// to optimize rendering as alpha blending can be disabled when there is no transparency.
	bool DETexture::HasAlpha() const
	{
		return Alpha;
	}
};
#endif