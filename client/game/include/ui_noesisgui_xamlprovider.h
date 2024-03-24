#ifndef __UI_NOESISGUI_XAMLPROVIDER_H__
#define __UI_NOESISGUI_XAMLPROVIDER_H__

#pragma once

namespace UINoesis
{
	class XamlProvider : public Noesis::XamlProvider
	{
	public:
		virtual Noesis::Ptr<Noesis::Stream> LoadXaml(const Noesis::Uri &uri) override;
	};
};

#endif