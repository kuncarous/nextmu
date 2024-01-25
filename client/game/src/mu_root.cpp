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
#include "mu_resourcesmanager.h"
#include "mu_animationsmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_skeletonmanager.h"
#include "mu_charactersmanager.h"
#include "mu_textureattachments.h"
#include "mu_model.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "res_renders.h"
#include "res_items.h"
#include "mu_math.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include "ui_noesisgui.h"
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
#include "ui_rmlui.h"
#endif

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

constexpr mu_double GameCycleTime = 1000.0 / 25.0; // 25 FPS

struct DemoSettings
{
	mu_utf8string Map = "data/worlds/areniltemple";
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

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
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

		if (MUResourcesManager::Load() == false)
		{
			mu_error("Failed to load resources.");
			return false;
		}

		if (MUCharactersManager::Load() == false)
		{
			mu_error("Failed to load characters.");
			return false;
		}

		if (MUTextureAttachments::Initialize() == false)
		{
			mu_error("Failed to initialize attachments.");
			return false;
		}

		if (MUAnimationsManager::Load() == false)
		{
			mu_error("Failed to load animations.");
			return false;
		}

		if (MURenderState::Initialize() == false)
		{
			mu_error("Failed to initialize render state.");
			return false;
		}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		if (UINoesis::Initialize() == false)
		{
			mu_error("Failed to initialize noesisgui.");
			return false;
		}
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
		if (UIRmlUI::Initialize() == false)
		{
			mu_error("Failed to initialize rmlui.");
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
		auto *window = MUWindow::GetWindow();
		if (window != nullptr)
		{
			SDL_RestoreWindow(window);
		}

		MUItemsManager::Destroy();
		MURendersManager::Destroy();
		MUBBoxRenderer::Destroy();
		MUModelRenderer::Destroy();
		MUSkeletonManager::Destroy();
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		UINoesis::Destroy();
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
		UIRmlUI::Destroy();
#endif
		MUResourcesManager::Destroy();
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

		const DemoSettings demo = LoadDemoSettings();

		std::unique_ptr<NEnvironment> environment(new NEnvironment());
		if (environment->Initialize() == false || environment->LoadTerrain(demo.Map) == false)
		{
			return;
		}

		environment->Reset(true);

		NModel *Model = MUResourcesManager::GetModel("monster_01");
		if (Model == nullptr)
		{
			return;
		}

		// Characters
		{
			auto characters = environment->GetCharacters();

			// Dark Knight
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 0,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 1,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 160,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 1);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 1);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 1);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 1);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 1);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Maces, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 5);
				characters->GenerateVirtualMeshToggle(entity);

				environment->GetController()->SetCharacter(entity);
			}

			// Dark Wizard
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 1,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 0,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 161,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Elf
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 2,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 2,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 162,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Magic Gladiator
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 3,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 3,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 163,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Dark Lord
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 4,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 4,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 164,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Summoner
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 5,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 5,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 165,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Rage Fighter
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 6,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 6,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 166,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Grow Lancer
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 7,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 7,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 167,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Rune Mage
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 8,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 8,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 168,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Slayer
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 9,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 9,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 169,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Gun Crasher
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 10,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 10,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 170,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// White Wizard
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 11,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 11,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 171,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Mage
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 12,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 12,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 172,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}

			// Illusion Knight
			{
				auto entity = characters->AddOrFind(
					TCharacter::Settings{
						.Key = 13,
						.Type = CharacterType::Character,
						.CharacterType = NCharacterType{
							.Class = 13,
							.SubClass = 0,
						},
						.AnimationsId = "player",
						.X = 173,
						.Y = 128,
						.Rotation = 0.0f,
					}
				);
				/*characters->AddAttachmentPartFromItem(entity, NPartType::Helm, EItemCategory::Helm, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Armor, EItemCategory::Armor, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Pants, EItemCategory::Pants, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Gloves, EItemCategory::Gloves, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::Boots, EItemCategory::Boots, 3);
				characters->AddAttachmentPartFromItem(entity, NPartType::ItemLeft, EItemCategory::Staffs, 5);
				characters->AddAttachmentPartFromItem(entity, NPartType::Wings, EItemCategory::Wings, 4);
				characters->GenerateVirtualMeshToggle(entity);*/
			}
		}

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
				/*auto *joints = environment->GetJoints();
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
							.Type = JointType::Thunder01_V7,
							.Position = position,
							.TargetPosition = position + glm::vec3(
								glm::linearRand(-80.0f, 80.0f),
								glm::linearRand(-80.0f, 80.0f),
								0.0f
							),
							.Scale = glm::linearRand(60.0f, 70.0f),
						}
					);
				}*/

				auto *particles = environment->GetParticles();
				constexpr mu_uint32 MaxParticlesPerType = glm::min(TParticle::MaxRenderCount / MaxParticleType, 40u);
				for (mu_uint32 type = 0; type < MaxParticleType; ++type)
				{
					for (mu_uint32 n = 0; n < MaxParticlesPerType; ++n)
					{
						particles->Create(
							NParticleData{
								.Layer = 0,
								.Type = static_cast<ParticleType>(type),
								.Position = glm::vec3(
									(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
									(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
									400.0f
								)
							}
						);
					}
				}

				/*
				for (mu_uint32 n = 0; n < 200; ++n)
				{
					particles->Create(
						NParticleData{
							.Layer = 0,
							.Type = static_cast<ParticleType>(static_cast<mu_uint32>(ParticleType::Flower01_V0) + glm::linearRand(0, 2)),
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
							.Type = ParticleType::Smoke05_V0,
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
							.Type = ParticleType::Smoke01_V0,
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
							.Type = ParticleType::TrueFire_Red_V5,
							.Position = glm::vec3(
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								(123.0f + glm::linearRand(-30.0f, 30.0f)) * TerrainScale,
								400.0f
							),
							.Scale = 2.8f
						}
					);
				}*/
			}

			environment->Reset();

			const auto device = MUGraphics::GetDevice();
			const auto swapchain = MUGraphics::GetSwapChain();
			const auto &swapchainDesc = swapchain->GetDesc();
			const auto immediateContext = MUGraphics::GetImmediateContext();

			mu_float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			auto *pRTV = swapchain->GetCurrentBackBufferRTV();
			auto *pDSV = swapchain->GetDepthBufferDSV();

			MURenderState::AttachEnvironment(environment.get());
			environment->Update();
			MUSkeletonManager::Update();

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
			UINoesis::Update();
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
			UIRmlUI::Update();
#endif

			if (MUConfig::GetEnableShadows())
			{
				MURenderState::SetRenderMode(NRenderMode::ShadowMap);
				environment->Render();
			}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
			UINoesis::RenderOffscreen();
#endif

			MURenderState::SetRenderMode(NRenderMode::Normal);
			MUGraphics::SetRenderTargetDesc(
				NRenderTargetDesc{
					.ColorFormat = swapchainDesc.ColorBufferFormat,
					.DepthStencilFormat = swapchainDesc.DepthBufferFormat,
				}
			);
			immediateContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			immediateContext->ClearRenderTarget(pRTV, clearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			immediateContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			environment->Render();

			ShaderResourcesBindingManager.MergeTemporaryShaderBindings();

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
			UINoesis::RenderOnscreen();
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
			UIRmlUI::RenderOnscreen();
#endif

			swapchain->Present(MUConfig::GetVerticalSync() ? 1u : 0u);
			MUGraphics::ClearTransactions();

			MUInput::ProcessKeys();

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				MURoot::OnPreEvent(&event);
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
				if (UINoesis::OnEvent(&event) == true) continue;
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
				if (UIRmlUI::OnEvent(&event) == true) continue;
#endif
				MURoot::OnEvent(&event);
			}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
			UIRmlUI::ReleaseGarbage();
#endif

			//OutputDebugStringA(fmt::format("particles count : {}\n", environment->GetParticles()->GetCount()).c_str());
			MUGlobalTimer::Wait();
			elapsedTime = MUGlobalTimer::GetElapsedFrametime();
			currentTime = MUGlobalTimer::GetFrametime();
		}

		environment->Destroy();

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
