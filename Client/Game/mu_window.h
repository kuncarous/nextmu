#ifndef __MU_WINDOW_H__
#define __MU_WINDOW_H__

#pragma once

namespace MUWindow
{
	const mu_boolean Initialize();
	void Destroy();

	SDL_Window* GetWindow();
};

#endif