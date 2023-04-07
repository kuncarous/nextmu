#include "stdafx.h"
#include "mu_graphics.h"
#include "mu_window.h"
#include "mu_config.h"

#include <bgfx/platform.h>
#include <SDL_syswm.h>

namespace MUGraphics
{
	mu_boolean Initialized = false;

	const mu_boolean Initialize()
	{
		auto *window = MUWindow::GetWindow();

		SDL_SysWMinfo wmi;
		SDL_VERSION(&wmi.version);
		if (SDL_GetWindowWMInfo(window, &wmi) == SDL_FALSE)
		{
			return false;
		}

		Initialized = true;

		bgfx::Init init;
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
		init.type = bgfx::RendererType::Direct3D11;
#else
		init.type = bgfx::RendererType::Count;
#endif
		init.vendorId = BGFX_PCI_ID_NONE;
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
		init.platformData.ndt = wmi.info.x11.display;
		init.platformData.nwh = (void *)(uintptr_t)wmi.info.x11.window;
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
		init.platformData.nwh = wmi.info.cocoa.window;
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
		init.platformData.nwh = wmi.info.win.window;
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
		init.platformData.nwh = wmi.info.android.window;
#endif
		init.resolution.width = MUConfig::GetWindowWidth();
		init.resolution.height = MUConfig::GetWindowHeight();
		init.resolution.reset = MUConfig::GetVerticalSync() ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
		bgfx::renderFrame();
		bgfx::init(init);

		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, MU::MakeRGBA(0, 0, 0, 255), 1.0f, 0);

#if NEXTMU_COMPILE_DEBUG == 1
		bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);
#endif

		FreeImage_Initialise(true);

		return true;
	}

	void Destroy()
	{
		if (!Initialized) return;
		bgfx::shutdown();
		FreeImage_DeInitialise();
	}

	const char *GetShaderFolder()
	{
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
		return "/windows/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
		return "/linux/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
		return "/macos/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
		return "/android/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
		return "/ios/";
#endif
	}

	const char *GetShaderExtension()
	{
		const auto renderer = bgfx::getRendererType();
		switch (renderer)
		{
		default: mu_error("Not supported renderer");
		case bgfx::RendererType::OpenGLES: return ".gles";
		case bgfx::RendererType::OpenGL: return ".gl";
		case bgfx::RendererType::Direct3D11: return ".d3d";
		case bgfx::RendererType::Direct3D12: return ".d3d";
		case bgfx::RendererType::Vulkan: return ".vk";
		}
	}
};