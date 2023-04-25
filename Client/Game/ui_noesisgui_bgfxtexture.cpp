#include "stdafx.h"
#include "ui_noesisgui_bgfxtexture.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	BGFXTexture::BGFXTexture(
		mu_uint32 width,
		mu_uint32 height,
		mu_uint32 levels,
		mu_uint32 sampleCount,
		bgfx::TextureFormat::Enum format,
		mu_boolean isInverted,
		mu_boolean hasAlphaChannel,
		bgfx::TextureHandle texture,
		mu_boolean flippedY
	) :
		Width(width),
		Height(height),
		Levels(levels),
		SampleCount(sampleCount),
		Format(format),
		Inverted(isInverted),
		Alpha(hasAlphaChannel),
		Texture(texture),
		FlippedY(flippedY)
	{

	}

	BGFXTexture::~BGFXTexture()
	{
		if (bgfx::isValid(Texture))
		{
			bgfx::destroy(Texture);
			Texture = BGFX_INVALID_HANDLE;
		}
	}

	/// Returns the width of the texture, in pixels
	uint32_t BGFXTexture::GetWidth() const
	{
		return Width;
	}

	/// Returns the height of the texture, in pixels
	uint32_t BGFXTexture::GetHeight() const
	{
		return Height;
	}

	uint32_t BGFXTexture::GetSampleCount() const
	{
		return SampleCount;
	}

	/// Returns true if the texture has mipmaps
	bool BGFXTexture::HasMipMaps() const
	{
		return Levels > 1;
	}

	/// Returns true when texture must be vertically inverted when mapped. This is true for surfaces
	/// on platforms (OpenGL) where texture V coordinate is zero at the "bottom of the texture"
	bool BGFXTexture::IsInverted() const
	{
		return Inverted;
	}

	/// Returns true if the texture has an alpha channel that is not completely white. Just a hint
	/// to optimize rendering as alpha blending can be disabled when there is no transparency.
	bool BGFXTexture::HasAlpha() const
	{
		return Alpha;
	}
};
#endif