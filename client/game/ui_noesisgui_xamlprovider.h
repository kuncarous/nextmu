#ifndef __UI_NOESISGUI_XAMLPROVIDER_H__
#define __UI_NOESISGUI_XAMLPROVIDER_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	class XamlProvider : public Noesis::XamlProvider
	{
	public:
		virtual Noesis::Ptr<Noesis::Stream> LoadXaml(const Noesis::Uri &uri) override;
	};
};
#endif

#endif