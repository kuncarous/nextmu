#include "stdafx.h"
#include "mu_window.h"
#include "mu_config.h"
#include <SDL.h>

namespace MUWindow
{
	SDL_Window* Window = nullptr;

	const mu_boolean Initialize()
	{
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
		constexpr mu_uint32 extraWindowFlags = SDL_WINDOW_ALLOW_HIGHDPI;
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
		constexpr mu_uint32 extraWindowFlags = SDL_WINDOW_ALLOW_HIGHDPI;
#else
		constexpr mu_uint32 extraWindowFlags = 0;
#endif

		const mu_int32 windowWidth = static_cast<mu_int32>(MUConfig::GetWindowWidth());
		const mu_int32 windowHeight = static_cast<mu_int32>(MUConfig::GetWindowHeight());

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
		Window = SDL_CreateWindow(NEXTMU_TITLE, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | extraWindowFlags);
#else
		Window = SDL_CreateWindow(NEXTMU_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | extraWindowFlags);
#endif
		if (Window == nullptr)
		{
			return false;
		}

		return true;
	}

	void Destroy()
	{
		if (Window != nullptr)
		{
			SDL_DestroyWindow(Window);
			Window = nullptr;
		}
	}

	SDL_Window* GetWindow()
	{
		return Window;
	}
};