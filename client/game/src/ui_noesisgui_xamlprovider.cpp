#include "mu_precompiled.h"
#include "ui_noesisgui_xamlprovider.h"
#include "ui_noesisgui_stream.h"
#include "ui_noesisgui_consts.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	Noesis::Ptr<Noesis::Stream> XamlProvider::LoadXaml(const Noesis::Uri &uri)
	{
		const mu_utf8string filename = SupportPathUTF8 + GetResourcesPath() + uri.Str();
		return Stream::Load(filename);
	}
};
#endif