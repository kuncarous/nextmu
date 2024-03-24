#ifndef __UI_NOESISGUI_DETEXTURE_H__
#define __UI_NOESISGUI_DETEXTURE_H__

#pragma once

namespace UINoesis
{
	class DETexture : public Noesis::Texture
	{
	public:
		DETexture(
			mu_uint32 width,
			mu_uint32 height,
			mu_uint32 levels,
			mu_uint32 sampleCount,
			mu_boolean isInverted,
			mu_boolean hasAlphaChannel,
			Diligent::TEXTURE_FORMAT format,
			Diligent::RefCntAutoPtr<Diligent::ITexture> texture,
			mu_boolean transitionRequired = false
		);
		~DETexture();

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

		Diligent::ITexture *GetTexture()
		{
			return Texture;
		}

		void SetRegionMapped(mu_boolean mapped)
		{
			RegionMapped = mapped;
		}

		mu_boolean GetRegionMapped() const
		{
			return RegionMapped;
		}

		void SetRequireTransition(mu_boolean require)
		{
			TransitionRequired = require;
		}

		mu_boolean GetRequireTransition() const
		{
			return TransitionRequired;
		}

	private:
		friend class DERenderDevice;
		NResourceId ResourceId;
		mu_uint32 Width;
		mu_uint32 Height;
		mu_uint32 Levels;
		mu_uint32 SampleCount;
		mu_boolean Inverted;
		mu_boolean Alpha;
		Diligent::TEXTURE_FORMAT Format;
		Diligent::RefCntAutoPtr<Diligent::ITexture> Texture;
		mu_boolean RegionMapped = false;
		mu_boolean TransitionRequired;
	};
};

#endif