#ifndef __UI_NOESISGUI_FONTPROVIDER_H__
#define __UI_NOESISGUI_FONTPROVIDER_H__

#pragma once

namespace UINoesis
{
	class FontProvider : public Noesis::CachedFontProvider
	{
	public:
		/// First time a font is requested from a folder, this function is invoked to give inheritors
		/// the opportunity to register faces found in that folder
		virtual void ScanFolder(const Noesis::Uri &folder) override;

		/// Returns a stream to a previously registered filename
		virtual Noesis::Ptr<Noesis::Stream> OpenFont(const Noesis::Uri &folder, const char *filename) const override;
	};
};

#endif