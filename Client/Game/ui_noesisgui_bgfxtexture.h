#ifndef __UI_NOESISGUI_BGFXTEXTURE_H__
#define __UI_NOESISGUI_BGFXTEXTURE_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	class BGFXTexture : public Noesis::Texture
	{
	public:
		BGFXTexture(
			mu_uint32 width,
			mu_uint32 height,
			mu_uint32 levels,
			mu_uint32 sampleCount,
			bgfx::TextureFormat::Enum format,
			mu_boolean isInverted,
			mu_boolean hasAlphaChannel,
			bgfx::TextureHandle texture
		);
		~BGFXTexture();

		/// Returns the width of the texture, in pixels
		virtual uint32_t GetWidth() const override;

		/// Returns the height of the texture, in pixels
		virtual uint32_t GetHeight() const override;

		/// Returns the sample count of the texture (MSAA sample counts)
		uint32_t GetSampleCount() const;

		/// Returns true if the texture has mipmaps
		virtual bool HasMipMaps() const override;

		/// Returns true when texture must be vertically inverted when mapped. This is true for surfaces
		/// on platforms (OpenGL) where texture V coordinate is zero at the "bottom of the texture"
		virtual bool IsInverted() const override;

		/// Returns true if the texture has an alpha channel that is not completely white. Just a hint
		/// to optimize rendering as alpha blending can be disabled when there is no transparency.
		virtual bool HasAlpha() const override;

		const bgfx::TextureHandle GetTexture() const
		{
			return Texture;
		}

	private:
		friend class BGFXRenderDevice;
		mu_uint32 Width;
		mu_uint32 Height;
		mu_uint32 Levels;
		mu_uint32 SampleCount;
		bgfx::TextureFormat::Enum Format;
		mu_boolean Inverted;
		mu_boolean Alpha;
		bgfx::TextureHandle Texture;
	};
};
#endif

#endif