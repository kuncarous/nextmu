#include "stdafx.h"
#include "ui_noesisgui_consts.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	const mu_utf8string ResourcesPath = "resources/";

	const mu_utf8string GetResourcesPath()
	{
		return ResourcesPath;
	}
};
#endif