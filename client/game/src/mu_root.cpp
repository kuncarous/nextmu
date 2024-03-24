#include "mu_precompiled.h"
#include "mu_root.h"
#include "mu_config.h"
#include "mu_window.h"
#include "mu_graphics.h"
#include "mu_capabilities.h"
#include "mu_physics.h"
#include "mu_timer.h"
#include "mu_input.h"
#include "mu_camera.h"
#include "mu_environment.h"
#include "mu_state.h"
#include "mu_renderstate.h"
#include "mu_controllerstate.h"
#include "mu_threadsmanager.h"
#include "mu_scenemanager.h"
#include "mu_resourcesmanager.h"
#include "mu_animationsmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_skeletonmanager.h"
#include "mu_charactersmanager.h"
#include "mu_eventsmanager.h"
#include "mu_model.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "mu_browsermanager.h"
#include "mu_webmanager.h"
#include "mu_webservermanager.h"
#include "mu_updatemanager.h"
#include "res_renders.h"
#include "res_items.h"
#include "mu_math.h"
#include "mu_angelscript.h"
#include "ui_noesisgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include "shared_binaryreader.h"
#include "mu_crypt.h"

namespace MURoot
{
	mu_boolean Quit = false;

	const mu_boolean Initialize(mu_int32 argc, mu_char **argv, void *instance, mu_int32 &exitResult)
	{
		boost::filesystem::path executablePath(argv[0]);
		ExecutablePath = executablePath.parent_path().string() + '/';
		std::replace(ExecutablePath.begin(), ExecutablePath.end(), '\\', '/');
		GamePath = boost::filesystem::current_path().string() + '/';
		std::replace(GamePath.begin(), GamePath.end(), '\\', '/');

		SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "1");
		SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
		SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

		if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0)
		{
			const mu_char* error = SDL_GetError();
			mu_error("Failed to initialize SDL ({}).", error);
			return false;
		}

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
		SupportPath = NXOperatingSystem::GetStorageSupportFilesPath();
		CachePath = NXOperatingSystem::GetStorageCacheFilesPath();
		UserPath = NXOperatingSystem::GetStorageUserFilesPath();
#endif

		if (MUConfig::Initialize() == false)
		{
			mu_error("Failed to load configuration.");
			return false;
		}

		if (MUThreadsManager::Initialize() == false)
		{
			mu_error("Failed to initialize threads.");
			return false;
		}

		if (MUWindow::Initialize() == false)
		{
			mu_error("Failed to initialize window.");
			return false;
		}

		if (MUGraphics::Initialize() == false)
		{
			mu_error("Failed to initialize graphics.");
			return false;
		}

		if (MUCapabilities::Configure() == false)
		{
			mu_error("Failed to configure capabilities.");
			return false;
		}

#if PHYSICS_ENABLED == 1
		if (MUPhysics::Initialize() == false)
		{
			mu_error("Failed to initialize physics.");
			return false;
		}
#endif

		if (MUSkeletonManager::Initialize() == false)
		{
			mu_error("Failed to initialize skeleton manager.");
			return false;
		}

		if (MUAngelScript::Initialize() == false)
		{
			mu_error("Failed to initialize scripting.");
			return false;
		}

#if NEXTMU_EMBEDDED_BROWSER == 1
		if (MUBrowserManager::Initialize(argc, argv, instance) == false)
		{
			mu_error("Failed to initialize embedded browser.");
			return false;
		}
#endif

#if NEXTMU_HTTP_SERVER == 1
		if (MUWebServerManager::Initialize() == false)
		{
			mu_error("Failed to initialize web server.");
			return false;
		}
#endif

		if (MUWebManager::Initialize() == false)
		{
			mu_error("Failed to initialize web manager.");
			return false;
		}

		if (MUSceneManager::Initialize() == false)
		{
			mu_error("Failed to initialize scene manager.");
			return false;
		}

		return true;
	}

	void Destroy()
	{
		auto *window = MUWindow::GetWindow();
		if (window != nullptr)
		{
			SDL_RestoreWindow(window);
		}

#if NEXTMU_HTTP_SERVER == 1
		MUWebServerManager::Destroy();
#endif
#if NEXTMU_EMBEDDED_BROWSER == 1
		MUBrowserManager::Destroy();
#endif
		MUSceneManager::Destroy();
		MURendersManager::Destroy();
		MUBBoxRenderer::Destroy();
		MUModelRenderer::Destroy();
		MUItemsManager::Destroy();
		UINoesis::Destroy();
		MUWebManager::Destroy();
		MUAngelScript::Destroy();
		MUSkeletonManager::Destroy();
#if PHYSICS_ENABLED == 1
		MUPhysics::Destroy();
#endif
		MURenderState::Destroy();
		MUGraphics::Destroy();
		MUWindow::Destroy();
		MUThreadsManager::Destroy();
		MUConfig::Destroy();

		SDL_Quit();
	}

	void OnPreEvent(const SDL_Event *event);
	void OnEvent(const SDL_Event* event);

	void Run()
	{
		static mu_double fpsCounterTime = 0.0;
		static mu_uint32 fpsCounterCount = 0;
		static mu_double fpsCounterLastValue = 0.0;
		static mu_uint32 fpsCounterLastCount = 0;

		MUGlobalTimer::Wait();
		mu_double elapsedTime = 0.0;
		mu_double currentTime = 0.0;

		static mu_double accumulatedTime = 0.0;
		while (!Quit)
		{
			accumulatedTime += elapsedTime;
			const mu_float updateTime = static_cast<mu_float>(elapsedTime / GameCycleTime);
			const mu_uint32 updateCount = static_cast<mu_uint32>(glm::floor(accumulatedTime / GameCycleTime)); // Calculate update count using fixed update time from MU (25 FPS)
			accumulatedTime -= static_cast<mu_double>(updateCount) * GameCycleTime; // Remove acummulated time

			MUState::SetTime(static_cast<mu_float>(currentTime), static_cast<mu_float>(elapsedTime));
			MUState::SetUpdate(updateTime, updateCount);
			MURenderState::Reset();
			MUSkeletonManager::Reset();
			MUWebManager::Run();

			fpsCounterTime += elapsedTime;
			++fpsCounterCount;

			if (fpsCounterTime >= 1000.0)
			{
				fpsCounterLastValue = (fpsCounterTime / (mu_double)fpsCounterCount);
				fpsCounterLastCount = fpsCounterCount;
				fpsCounterCount = 0;
				fpsCounterTime = 0.0;
			}

			const auto device = MUGraphics::GetDevice();
			const auto swapchain = MUGraphics::GetSwapChain();
			const auto &swapchainDesc = swapchain->GetDesc();
			const auto immediateContext = MUGraphics::GetImmediateContext();

			MURenderState::SetRenderSize(swapchainDesc.Width, swapchainDesc.Height);

			MUSceneManager::Update();
			auto *scene = MUSceneManager::GetScene();
			if (scene != nullptr)
			{
				scene->Run();
			}

#if NEXTMU_EMBEDDED_BROWSER == 1
			MUBrowserManager::Update();
			MUBrowserManager::Render();
#endif
			swapchain->Present(MUConfig::GetVerticalSync() ? 1u : 0u);
			MUGraphics::ClearTransactions();

			MUInput::ProcessKeys();
			MUEventsManager::Process();

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				MURoot::OnPreEvent(&event);
#if NEXTMU_EMBEDDED_BROWSER == 1
				if (MUBrowserManager::ProcessEvent(&event) == true) continue;
#endif
				if (UINoesis::OnEvent(&event) == true) continue;
				MURoot::OnEvent(&event);
			}

			MUGlobalTimer::Wait();
			elapsedTime = MUGlobalTimer::GetElapsedFrametime();
			currentTime = MUGlobalTimer::GetFrametime();
		}

		mu_trace_info("Destroying game");
	}

	void OnPreEvent(const SDL_Event *event)
	{
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
			{
				if (event->motion.which == SDL_TOUCH_MOUSEID) return;
				const mu_int32 mx = MUInput::GetRealPixelSize(event->motion.x);
				const mu_int32 my = MUInput::GetRealPixelSize(event->motion.y);
				MUInput::SetMousePosition(mx, my);
			}
			break;

		case SDL_QUIT:
			{
				Quit = true;
			}
			break;
		}
	}

	void OnEvent(const SDL_Event* event)
	{
		switch (event->type)
		{
		case SDL_MOUSEBUTTONDOWN:
			{
				switch (event->button.button)
				{
				case SDL_BUTTON_LEFT:
					{
						if (event->button.clicks < 2)
						{
							MUInput::SetMouseButton(MOUSE_BUTTON_LEFT, MOUSE_STATE_CLICK);
						}
						else
						{
							MUInput::SetMouseButton(MOUSE_BUTTON_LEFT, MOUSE_STATE_DOUBLECLICK);
						}
					}
					break;
				case SDL_BUTTON_MIDDLE:
					{
						if (event->button.clicks < 2)
						{
							MUInput::SetMouseButton(MOUSE_BUTTON_MIDDLE, MOUSE_STATE_CLICK);
						}
						else
						{
							MUInput::SetMouseButton(MOUSE_BUTTON_MIDDLE, MOUSE_STATE_DOUBLECLICK);
						}
					}
					break;
				case SDL_BUTTON_RIGHT:
					{
						if (event->button.clicks < 2)
						{
							MUInput::SetMouseButton(MOUSE_BUTTON_RIGHT, MOUSE_STATE_CLICK);
						}
						else
						{
							MUInput::SetMouseButton(MOUSE_BUTTON_RIGHT, MOUSE_STATE_DOUBLECLICK);
						}
					}
					break;
				}
			}
			break;

		case SDL_MOUSEBUTTONUP:
			{
				switch (event->button.button)
				{
				case SDL_BUTTON_LEFT:
					{
						MUInput::SetMouseButton(MOUSE_BUTTON_LEFT, MOUSE_STATE_NONE);
					}
					break;
				case SDL_BUTTON_MIDDLE:
					{
						MUInput::SetMouseButton(MOUSE_BUTTON_MIDDLE, MOUSE_STATE_NONE);
					}
					break;
				case SDL_BUTTON_RIGHT:
					{
						MUInput::SetMouseButton(MOUSE_BUTTON_RIGHT, MOUSE_STATE_NONE);
					}
					break;
				}
			}
			break;

		case SDL_MOUSEWHEEL:
			{
				MUInput::AddMouseWheel(event->wheel.y);
			}
			break;

		case SDL_KEYDOWN:
			{
				MUInput::SetKeyDown(event->key.keysym.scancode);
			}
			break;

		case SDL_KEYUP:
			{
				MUInput::SetKeyUp(event->key.keysym.scancode);
			}
			break;
		}
	}
};
