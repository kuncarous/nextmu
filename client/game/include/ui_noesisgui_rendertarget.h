#ifndef __UI_NOESISGUI_RENDERTARGET_H__
#define __UI_NOESISGUI_RENDERTARGET_H__

#pragma once

namespace UINoesis
{
	class DETexture;
	class DERenderTarget : public Noesis::RenderTarget
	{
	public:
		DERenderTarget(
			Noesis::Ptr<DETexture> color,
			Diligent::RefCntAutoPtr<Diligent::ITexture> colorMSAA,
			Diligent::RefCntAutoPtr<Diligent::ITexture> depthStencil
		);
		virtual ~DERenderTarget();

		virtual Noesis::Texture *GetTexture() override;
		bool IsMSAA() const;
		bool HasDepthStencil() const;

	protected:
		friend class DERenderDevice;
		Noesis::Ptr<DETexture> ColorTexture;
		Diligent::RefCntAutoPtr<Diligent::ITexture> ColorMSAATexture;
		Diligent::RefCntAutoPtr<Diligent::ITexture> DepthStencilTexture;
	};
};

#endif