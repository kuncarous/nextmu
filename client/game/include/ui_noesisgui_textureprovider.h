#ifndef __UI_NOESISGUI_TEXTUREPROVIDER_H__
#define __UI_NOESISGUI_TEXTUREPROVIDER_H__

#pragma once

namespace UINoesis
{
	class TextureProvider : public Noesis::TextureProvider
	{
	public:
		/// Returns metadata for the texture at the given URI or empty rectangle if texture is not found
		virtual Noesis::TextureInfo GetTextureInfo(const Noesis::Uri &uri) override;

		/// Returns a texture compatible with the given device or null if texture is not found
		virtual Noesis::Ptr<Noesis::Texture> LoadTexture(const Noesis::Uri &uri, Noesis::RenderDevice *device) override;
	};
};

#endif