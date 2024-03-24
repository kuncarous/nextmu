#include "mu_precompiled.h"
#include "ui_noesisgui_xamlprovider.h"
#include "ui_noesisgui_stream.h"
#include "ui_noesisgui_consts.h"

namespace UINoesis
{
	Noesis::Ptr<Noesis::Stream> XamlProvider::LoadXaml(const Noesis::Uri &uri)
	{
		const mu_utf8string filename = SupportPath + GetResourcesPath() + uri.Str();
		return Stream::Load(filename);
	}
};