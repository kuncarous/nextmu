#include "stdafx.h"
#include "ui_noesisgui_bgfxrendertarget.h"
#include "ui_noesisgui_bgfxtexture.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	BGFXRenderTarget::BGFXRenderTarget(
		Noesis::Ptr<BGFXTexture> color,
		bgfx::TextureHandle colorMSAA,
		bgfx::TextureHandle depthStencil,
		bgfx::FrameBufferHandle framebuffer
	) :
		ColorTexture(std::move(color)),
		ColorMSAATexture(colorMSAA),
		DepthStencilTexture(depthStencil),
		Framebuffer(framebuffer)
	{
	}

	BGFXRenderTarget::~BGFXRenderTarget()
	{
		if (bgfx::isValid(Framebuffer))
		{
			bgfx::destroy(Framebuffer);
			Framebuffer = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(ColorMSAATexture))
		{
			bgfx::destroy(ColorMSAATexture);
			ColorMSAATexture = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(DepthStencilTexture))
		{
			bgfx::destroy(DepthStencilTexture);
			DepthStencilTexture = BGFX_INVALID_HANDLE;
		}
	}

	Noesis::Texture *BGFXRenderTarget::GetTexture()
	{
		return ColorTexture.GetPtr();
	}

	bool BGFXRenderTarget::HasDepthStencil() const
	{
		return bgfx::isValid(DepthStencilTexture);
	}
};
#endif