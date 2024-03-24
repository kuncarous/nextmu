#include "mu_precompiled.h"
#include "ui_noesisgui_rendertarget.h"
#include "ui_noesisgui_texture.h"

namespace UINoesis
{
	DERenderTarget::DERenderTarget(
		Noesis::Ptr<DETexture> color,
		Diligent::RefCntAutoPtr<Diligent::ITexture> colorMSAA,
		Diligent::RefCntAutoPtr<Diligent::ITexture> depthStencil
	) :
		ColorTexture(std::move(color)),
		ColorMSAATexture(colorMSAA),
		DepthStencilTexture(depthStencil)
	{
	}

	DERenderTarget::~DERenderTarget()
	{}

	Noesis::Texture *DERenderTarget::GetTexture()
	{
		return ColorTexture.GetPtr();
	}

	bool DERenderTarget::IsMSAA() const
	{
		return ColorMSAATexture != nullptr;
	}

	bool DERenderTarget::HasDepthStencil() const
	{
		return DepthStencilTexture != nullptr;
	}
};