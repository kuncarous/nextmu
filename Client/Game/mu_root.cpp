#include "stdafx.h"
#include "mu_root.h"
#include "mu_config.h"
#include "mu_window.h"
#include "mu_graphics.h"
#include "mu_timer.h"
#include "mu_input.h"
#include "mu_camera.h"
#include "mu_terrain.h"
#include "mu_state.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_skeletonmanager.h"
#include "mu_modelrenderer.h"
#include "mu_model.h"

#include "ui_ultralight.h"
#include "ui_noesisgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_bgfx.h"
#include "backends/imgui_impl_sdl.h"

constexpr mu_double GameCycleTime = 1000.0 / 25.0; // 25 FPS

struct DemoSettings
{
	mu_utf8string Map = "data/worlds/lorencia";
	mu_float X = 160.0f;
	mu_float Y = 116.5f;
	mu_float Z = 1282.0f;
	mu_float CX = 140.0f;
	mu_float CY = 137.0f;
};

const DemoSettings LoadDemoSettings()
{
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, "demo.json", "rb") == false)
	{
		return DemoSettings();
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
	if (!!jsonBuffer)
	{
		SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
	}
	SDL_RWclose(fp);

	if (!jsonBuffer)
	{
		return DemoSettings();
	}

	const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
	jsonBuffer.reset();
	auto document = nlohmann::json::parse(inputBuffer.c_str());
	if (document.is_discarded() == true)
	{
		return DemoSettings();
	}

	DemoSettings settings;

	settings.Map = document["map"].get<mu_utf8string>();

	settings.X = document["x"].get<mu_float>();
	settings.Y = document["y"].get<mu_float>();
	settings.Z = document["z"].get<mu_float>();

	settings.CX = document["cx"].get<mu_float>();
	settings.CY = document["cy"].get<mu_float>();

	return settings;
}

namespace MURoot
{
	mu_boolean Quit = false;

	const bool Initialize()
	{
		if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
		{
			const mu_char* error = SDL_GetError();
			mu_error("Failed to initialize SDL ({}).", error);
			return false;
		}

		SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "1");

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
		SupportPathUTF8 = NXOperatingSystem::GetStorageSupportFilesPath();
		SupportPathUnicode = ConvertToUnicodeString(SupportPathUTF8);
		CachePathUTF8 = NXOperatingSystem::GetStorageCacheFilesPath();
		CachePathUnicode = ConvertToUnicodeString(CachePathUTF8);
		UserPathUTF8 = NXOperatingSystem::GetStorageUserFilesPath();
		UserPathUnicode = ConvertToUnicodeString(UserPathUTF8);
#endif

		if (MUConfig::Initialize() == false)
		{
			mu_error("Failed to load configuration.");
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

		if (MUSkeletonManager::Initialize() == false)
		{
			mu_error("Failed to initialize skeleton manager.");
			return false;
		}

		if (MUResourcesManager::Load() == false)
		{
			mu_error("Failed to load resources.");
			return false;
		}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
		if (UIUltralight::Initialize() == false)
		{
			mu_error("Failed to initialize ultralight.");
			return false;
		}
#endif

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		if (UINoesis::Initialize() == false)
		{
			mu_error("Failed to initialize ultralight.");
			return false;
		}
#endif

		if (MUModelRenderer::Initialize() == false)
		{
			mu_error("Failed to initialize model renderer.");
			return false;
		}

		return true;
	}

	void Destroy()
	{
		MUModelRenderer::Destroy();
		MUSkeletonManager::Destroy();
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
		UIUltralight::Destroy();
#endif
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		UINoesis::Destroy();
#endif
		MUResourcesManager::Destroy();
		MUGraphics::Destroy();
		MUWindow::Destroy();
		MUConfig::Destroy();

		SDL_Quit();
	}

	void OnEvent(const SDL_Event* event);

	struct MobInfo
	{
		mu_uint16 PriorAction = 0;
		mu_uint16 CurrentAction = 0;
		mu_float PriorFrame = 0.0f;
		mu_float CurrentFrame = 0.0f;
	};

	std::vector<MobInfo> MobsInfo;
	std::vector<NSkeletonInstance> MobsInstance;
	std::vector<mu_uint32> MobsBonesOffset;

	const mu_uint32 GetMobsCount()
	{
		return static_cast<mu_uint32>(MobsInstance.size());
	}

	void AddMob()
	{
		if (MobsInstance.size() >= 600) return;

		NSkeletonInstance instance;
		instance.SetParent(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			1.0f
		);

		MobsInfo.push_back(MobInfo());
		MobsInstance.push_back(instance);
		MobsBonesOffset.push_back(0);
	}

	void RemoveMob()
	{
		if (MobsInstance.size() == 0) return;

		MobsInfo.pop_back();
		MobsInstance.pop_back();
		MobsBonesOffset.pop_back();
	}

	void Run()
	{
		static mu_double fpsCounterTime = 0.0;
		static mu_uint32 fpsCounterCount = 0;
		static mu_double fpsCounterLastValue = 0.0;
		static mu_uint32 fpsCounterLastCount = 0;

		MUGlobalTimer::Wait();
		mu_double elapsedTime = 0.0;
		mu_double currentTime = 0.0;

		const DemoSettings demo = LoadDemoSettings();

		NCamera camera;
		camera.SetMode(NCameraMode::Directional);
		camera.SetEye(glm::vec3(demo.X * TerrainScale, demo.Y * TerrainScale, demo.Z));
		camera.SetAngle(glm::vec3(glm::radians(demo.CX), glm::radians(demo.CY), glm::radians(0.0f)));
		camera.SetUp(glm::vec3(0.0f, 0.0f, 1.0f));

		NTerrain terrain;
		if (terrain.Initialize(demo.Map) == false)
		{
			return;
		}

		AddMob();

		const NModel *Model = MUResourcesManager::GetModel("monster_01");
		if (Model == nullptr)
		{
			return;
		}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_IMGUI
		ImGuiIO &io = ImGui::GetIO();
#endif

		static mu_double accumulatedTime = 0.0;
		static glm::mat4 view, projection;
		while (!Quit)
		{
			accumulatedTime += elapsedTime;
			const mu_float updateTime = static_cast<mu_float>(elapsedTime / GameCycleTime);
			const mu_uint32 updateCount = static_cast<mu_uint32>(updateTime); // Calculate update count using fixed update time from MU (25 FPS)
			accumulatedTime -= static_cast<mu_double>(updateCount) * GameCycleTime; // Remove acummulated time

			MUState::SetTime(static_cast<mu_float>(currentTime), static_cast<mu_float>(elapsedTime));
			MUState::SetUpdate(updateTime, updateCount);
			MUSkeletonManager::Reset();

			fpsCounterTime += elapsedTime;
			++fpsCounterCount;

			if (fpsCounterTime >= 1000.0)
			{
				fpsCounterLastValue = (fpsCounterTime / (mu_double)fpsCounterCount);
				fpsCounterLastCount = fpsCounterCount;
				fpsCounterCount = 0;
				fpsCounterTime = 0.0;
			}

			if (updateCount > 0)
			{
				terrain.Reset();
			}

			camera.Update();

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_IMGUI
			ImGui_Implbgfx_NewFrame();
			ImGui_ImplSDL2_NewFrame();

			ImGui::NewFrame();

			//ImGui::ShowDemoWindow(); // your drawing here

			ImGui::Begin("Debug");
			{
				ImGui::SetWindowSize(ImVec2(180, 160));
				ImGui::Text(fmt::format("FPS : {} ({}ms)", fpsCounterLastCount, static_cast<mu_uint32>(fpsCounterLastValue)).c_str());
				ImGui::Text(fmt::format("Mobs : {}", GetMobsCount()).c_str());
				mu_boolean isFpsLimited = MUGlobalTimer::IsFpsLimited();
				if (ImGui::Checkbox("FPS Limit", &isFpsLimited))
				{
					MUGlobalTimer::ToggleLimitFPS();
				}
				if (ImGui::Button("Increase Mobs"))
				{
					AddMob();
				}
				if (ImGui::Button("Decrease Mobs"))
				{
					RemoveMob();
				}
			}
			ImGui::End();

			ImGui::Render();
#endif

			const mu_uint32 mobsCount = GetMobsCount();
			for (mu_uint32 n = 0; n < mobsCount; ++n)
			{
				auto &mobInfo = MobsInfo[n];
				auto &skeleton = MobsInstance[n];
				skeleton.PlayAnimation(Model, mobInfo.CurrentAction, mobInfo.PriorAction, mobInfo.CurrentFrame, mobInfo.PriorFrame, Model->GetPlaySpeed() * MUState::GetUpdateTime());
				skeleton.Animate(
					Model,
					{
						.Action = mobInfo.CurrentAction,
						.Frame = mobInfo.CurrentFrame,
					},
					{
						.Action = mobInfo.PriorAction,
						.Frame = mobInfo.PriorFrame,
					},
					glm::vec3(0.0f, 0.0f, 0.0f)
					);
			}

			const auto windowWidth = MUConfig::GetWindowWidth();
			const auto windowHeight = MUConfig::GetWindowHeight();

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(windowWidth), static_cast<uint16_t>(windowHeight));
			bgfx::touch(0);

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_IMGUI
			ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
#endif

			auto caps = bgfx::getCaps();

			view = camera.GetView();
			projection = glm::perspectiveFov(
				glm::radians(35.0f),
				static_cast<mu_float>(windowWidth),
				static_cast<mu_float>(windowHeight),
				50.0f,
				caps->homogeneousDepth
				? 2000.0f
				: 100000.0f
			);

			bgfx::setViewTransform(0, glm::value_ptr(view), glm::value_ptr(projection));

			if (updateCount > 0)
			{
				terrain.Update();
			}

			terrain.ConfigureUniforms();
			terrain.Render();

			MUModelRenderer::AttachTerrain(&terrain);

			for (mu_uint32 n = 0; n < mobsCount; ++n)
			{
				auto &skeleton = MobsInstance[n];
				MobsBonesOffset[n] = skeleton.Upload();
			}

			MUSkeletonManager::Update();

			mu_float mx = 0.0f, my = 0.0f;
			for (mu_uint32 n = 0; n < mobsCount; ++n)
			{
				const auto bonesOffset = MobsBonesOffset[n];
				if (bonesOffset == NInvalidUInt32) continue;
				MUModelRenderer::RenderBody(Model, bonesOffset, glm::vec3((100.0f + mx) * TerrainScale, (100.0f + my) * TerrainScale, 800.0f), 1.0f);
				mx += 2.0f;
				if (mx >= 100.0f)
				{
					mx = 0.0f;
					my += 2.0f;
				}
			}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
			UIUltralight::UpdateLogic();
			UIUltralight::RenderOneFrame();
			UIUltralight::Present();
#endif

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
			UINoesis::Update();
			UINoesis::Render();
#endif

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			MUInput::ProcessKeys();

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				MURoot::OnEvent(&event);
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_IMGUI
				ImGui_ImplSDL2_ProcessEvent(&event);
#endif
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
				if (UIUltralight::OnEvent(&event) == true) continue;
#endif
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
				if (UINoesis::OnEvent(&event) == true) continue;
#endif
			}

			MUGlobalTimer::Wait();
			elapsedTime = MUGlobalTimer::GetElapsedFrametime();
			currentTime = MUGlobalTimer::GetFrametime();
		}

		mu_trace_info("Destroying game");
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

		case SDL_QUIT:
			{
				Quit = true;
			}
			break;
		}
	}
};