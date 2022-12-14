#ifndef __UI_NOESISGUI_BGFXRENDERTARGET_H__
#define __UI_NOESISGUI_BGFXRENDERTARGET_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	class BGFXTexture;
	class BGFXRenderTarget : public Noesis::RenderTarget
	{
	public:
		BGFXRenderTarget(
			Noesis::Ptr<BGFXTexture> color,
			bgfx::TextureHandle colorMSAA,
			bgfx::TextureHandle depthStencil,
			bgfx::FrameBufferHandle framebuffer
		);
		virtual ~BGFXRenderTarget();

		virtual Noesis::Texture *GetTexture() override;
		bool HasDepthStencil() const;

	protected:
		friend class BGFXRenderDevice;
		Noesis::Ptr<BGFXTexture> ColorTexture;
		bgfx::TextureHandle ColorMSAATexture;
		bgfx::TextureHandle DepthStencilTexture;
		bgfx::FrameBufferHandle Framebuffer;
	};
};
#endif

#endif