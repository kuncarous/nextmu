#ifndef __UI_NOESISGUI_H__
#define __UI_NOESISGUI_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	const mu_boolean Initialize();
	void Destroy();

	void Update();
	void Render();

	const mu_boolean OnEvent(const SDL_Event *event);
};
#endif

#endif