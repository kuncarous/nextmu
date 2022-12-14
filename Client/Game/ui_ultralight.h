#ifndef __UI_ULTRALIGHT_H__
#define __UI_ULTRALIGHT_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
namespace UIUltralight
{
	const mu_boolean Initialize();
	void Destroy();

	void UpdateLogic();
	void RenderOneFrame();

	void Present();

	const mu_boolean OnEvent(const SDL_Event *event);
};
#endif

#endif