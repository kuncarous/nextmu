#ifndef __UI_NOESISGUI_CONSTS_H__
#define __UI_NOESISGUI_CONSTS_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	constexpr mu_uint32 RenderView = 255u;
	const mu_utf8string GetResourcesPath();
};
#endif

#endif