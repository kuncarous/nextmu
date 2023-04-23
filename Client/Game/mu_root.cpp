#include "stdafx.h"
#include "mu_root.h"
#include "mu_config.h"
#include "mu_window.h"
#include "mu_angelscript.h"
#include "mu_graphics.h"
#include "mu_capabilities.h"
#include "mu_physics.h"
#include "mu_timer.h"
#include "mu_input.h"
#include "mu_camera.h"
#include "mu_environment.h"
#include "mu_state.h"
#include "mu_renderstate.h"
#include "mu_threadsmanager.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_skeletonmanager.h"
#include "mu_model.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "res_renders.h"
#include "res_items.h"

#include "ui_noesisgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

constexpr mu_double GameCycleTime = 1000.0 / 25.0; // 25 FPS

struct DemoSettings
{
	mu_utf8string Map = "data/worlds/lorencia";
	mu_float X = 160.0f;
	mu_float Y = 116.5f;
	mu_float Z = 1282.0f;
	mu_float CX = 140.0f;
	mu_float CY = 137.0f;
	mu_uint32 MonstersCount = 1u;
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
	SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
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

	if (document.contains("monstersCount"))
	{
		settings.MonstersCount = document["monstersCount"].get<mu_uint32>();
	}

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

		if (MUAngelScript::Initialize() == false)
		{
			mu_error("Failed to initialize angelscript.");
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

		if (MURenderState::Initialize() == false)
		{
			mu_error("Failed to initialize render state.");
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

		if (MUResourcesManager::Load() == false)
		{
			mu_error("Failed to load resources.");
			return false;
		}

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

		if (MUBBoxRenderer::Initialize() == false)
		{
			mu_error("Failed to initialize model renderer.");
			return false;
		}

		if (MURendersManager::Initialize() == false)
		{
			return false;
		}

		if (MUItemsManager::Initialize() == false)
		{
			return false;
		}

		return true;
	}

	void Destroy()
	{
		MUItemsManager::Destroy();
		MURendersManager::Destroy();
		MUBBoxRenderer::Destroy();
		MUModelRenderer::Destroy();
		MUSkeletonManager::Destroy();
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		UINoesis::Destroy();
#endif
		MUResourcesManager::Destroy();
#if PHYSICS_ENABLED == 1
		MUPhysics::Destroy();
#endif
		MURenderState::Destroy();
		MUGraphics::Destroy();
		MUAngelScript::Destroy();
		MUWindow::Destroy();
		MUThreadsManager::Destroy();
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

		static NCamera camera;
		camera.SetMode(NCameraMode::Directional);
		camera.SetEye(glm::vec3(demo.X * TerrainScale, demo.Y * TerrainScale, demo.Z));
		camera.SetAngle(glm::vec3(glm::radians(demo.CX), glm::radians(demo.CY), glm::radians(0.0f)));
		camera.SetUp(glm::vec3(0.0f, 0.0f, 1.0f));

		std::unique_ptr<NEnvironment> environment(new NEnvironment());
		if (environment->Initialize() == false || environment->LoadTerrain(demo.Map) == false)
		{
			return;
		}

		environment->Reset(true);

		for (mu_uint32 n = 0; n < demo.MonstersCount; ++n)
		{
			AddMob();
		}

		NModel *Model = MUResourcesManager::GetModel("monster_01");
		if (Model == nullptr)
		{
			return;
		}

		auto characters = environment->GetCharacters();
		characters->AddOrFind(
			TCharacter::Settings{
				.Key = 0,
				.Type = CharacterType::Character,
				.X = 160,
				.Y = 123,
				.Rotation = 0.0f,
			}
		);

		static mu_double accumulatedTime = 0.0;
		static cglm::mat4 view, projection, frustumProjection;
		while (!Quit)
		{
			accumulatedTime += elapsedTime;
			const mu_float updateTime = static_cast<mu_float>(accumulatedTime / GameCycleTime);
			const mu_uint32 updateCount = static_cast<mu_uint32>(updateTime); // Calculate update count using fixed update time from MU (25 FPS)
			accumulatedTime -= static_cast<mu_double>(updateCount) * GameCycleTime; // Remove acummulated time

			MUState::SetTime(static_cast<mu_float>(currentTime), static_cast<mu_float>(elapsedTime));
			MUState::SetUpdate(glm::floor(updateTime), updateCount);
			MURenderState::Reset();
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
				auto *joints = environment->GetJoints();
				for (mu_uint32 n = 0; n < 60; ++n)
				{
					const glm::vec3 position = glm::vec3(
						(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
						(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
						800.0f
					);
					joints->Create(
						NJointData {
							.Layer = 0,
							.Type = JointType::Thunder1_V7,
							.Position = position,
							.TargetPosition = position + glm::vec3(
								glm::linearRand(-80.0f, 80.0f),
								glm::linearRand(-80.0f, 80.0f),
								0.0f
							),
							.Scale = glm::linearRand(60.0f, 70.0f),
						}
					);
				}/**/

				auto *particles = environment->GetParticles();
				for (mu_uint32 n = 0; n < 200; ++n)
				{
					particles->Create(
						NParticleData {
							.Layer = 0,
							.Type = ParticleType::Bubble_V0,
							.Position = glm::vec3(
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								400.0f
							)
						}
					);
				}
				for (mu_uint32 n = 0; n < 200; ++n)
				{
					particles->Create(
						NParticleData {
							.Layer = 0,
							.Type = ParticleType::Smoke_V0,
							.Position = glm::vec3(
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								400.0f
							)
						}
					);
				}
				for (mu_uint32 n = 0; n < 200; ++n)
				{
					particles->Create(
						NParticleData {
							.Layer = 0,
							.Type = ParticleType::TrueFire_V5,
							.Position = glm::vec3(
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								400.0f
							),
							.Scale = 2.8f
						}
					);
				}/**/
			}

			camera.Update();
			environment->Reset();

			const mu_uint32 mobsCount = GetMobsCount();
			for (mu_uint32 n = 0; n < mobsCount; ++n)
			{
				auto &mobInfo = MobsInfo[n];
				auto &skeleton = MobsInstance[n];
				Model->PlayAnimation(mobInfo.CurrentAction, mobInfo.PriorAction, mobInfo.CurrentFrame, mobInfo.PriorFrame, Model->GetPlaySpeed() * MUState::GetUpdateTime());
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

			const auto device = MUGraphics::GetDevice();
			const auto swapchain = MUGraphics::GetSwapChain();
			const auto immediateContext = MUGraphics::GetImmediateContext();
			MURenderState::SetImmediateContext(immediateContext);

			mu_float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			auto *pRTV = swapchain->GetCurrentBackBufferRTV();
			auto *pDSV = swapchain->GetDepthBufferDSV();
			immediateContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			immediateContext->ClearRenderTarget(pRTV, clearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			immediateContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			camera.GetView(view);

			constexpr mu_float HomogeneousFar = 5000.0f;
			constexpr mu_float NonHomogeneousFar = 50000.0f;
			const mu_float aspect = static_cast<mu_float>(windowWidth) / static_cast<mu_float>(windowHeight);
			if (MUCapabilities::IsHomogeneousDepth())
			{
				cglm::glm_perspective_rh_no(
					cglm::glm_rad(35.0f),
					aspect,
					50.0f,
					HomogeneousFar,
					projection
				);
				cglm::glm_perspective_rh_no(
					cglm::glm_rad(35.0f),
					aspect,
					50.0f,
					HomogeneousFar,
					frustumProjection
				);
			}
			else
			{
				cglm::glm_perspective_rh_zo(
					cglm::glm_rad(35.0f),
					aspect,
					50.0f,
					NonHomogeneousFar,
					projection
				);
				cglm::glm_perspective_rh_no(
					cglm::glm_rad(35.0f),
					aspect,
					50.0f,
					NonHomogeneousFar,
					frustumProjection
				);
			}

			camera.GenerateFrustum(view, frustumProjection);

			MURenderState::SetViewTransform(view, projection);
			MURenderState::AttachCamera(&camera);
			MURenderState::AttachEnvironment(environment.get());

			environment->Update();

			for (mu_uint32 n = 0; n < mobsCount; ++n)
			{
				auto &skeleton = MobsInstance[n];
				MobsBonesOffset[n] = skeleton.Upload();
			}

			MUSkeletonManager::Update();

			environment->Render();

			mu_float mx = 0.0f, my = 0.0f;
			for (mu_uint32 n = 0; n < mobsCount; ++n)
			{
				const auto bonesOffset = MobsBonesOffset[n];
				if (bonesOffset == NInvalidUInt32) continue;
				const glm::vec3 position = glm::vec3((100.0f + mx) * TerrainScale, (100.0f + my) * TerrainScale, 800.0f);
				const mu_float scale = 1.0f;
				const NRenderConfig config = {
					.BoneOffset = bonesOffset,
					.BodyOrigin = position,
					.BodyScale = scale,
					.EnableLight = true,
					.BodyLight = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				};
				MUModelRenderer::RenderBody(MobsInstance[n], Model, config);
				mx += 2.0f;
				if (mx >= 100.0f)
				{
					mx = 0.0f;
					my += 2.0f;
				}
			}

			MUGraphics::GetRenderManager()->Execute(immediateContext);

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
			UINoesis::Update();
			UINoesis::Render();
#endif

			swapchain->Present(MUConfig::GetVerticalSync() ? 1u : 0u);

			MUInput::ProcessKeys();

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				MURoot::OnEvent(&event);
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
				if (UINoesis::OnEvent(&event) == true) continue;
#endif
			}

			MUGlobalTimer::Wait();
			elapsedTime = MUGlobalTimer::GetElapsedFrametime();
			currentTime = MUGlobalTimer::GetFrametime();
		}

		environment->Destroy();

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