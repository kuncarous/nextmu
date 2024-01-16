#include "mu_precompiled.h"
#include "ui_noesisgui_fontprovider.h"
#include "ui_noesisgui_stream.h"
#include "ui_noesisgui_consts.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	void FontProvider::ScanFolder(const Noesis::Uri &folder)
	{
		const mu_utf8string path = SupportPathUTF8 + GetResourcesPath() + folder.Str();
		std::vector<mu_utf8string> fileList;
		if (NXOperatingSystem::EnumerateFiles(path, fileList) == false)
		{
			return;
		}

		for (const auto &filename : fileList)
		{
			const auto extensionAt = filename.find_last_of('.');
			if (extensionAt == mu_utf8string::npos || extensionAt + 1 >= filename.size()) continue;
			const mu_utf8string extension = filename.substr(extensionAt + 1);
			if (
				extension == "ttf" ||
				extension == "otf" ||
				extension == "ttc"
				)
			{
				RegisterFont(folder, filename.c_str());
			}
		}
	}

	Noesis::Ptr<Noesis::Stream> FontProvider::OpenFont(const Noesis::Uri &folder, const char *filename) const
	{
		const mu_utf8string path = SupportPathUTF8 + GetResourcesPath() + folder.Str() + "/" + filename;
		return Stream::Load(path);
	}
};
#endif