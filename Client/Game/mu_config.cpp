#include "stdafx.h"
#include "mu_config.h"

namespace MUConfig
{
	mu_boolean WindowMode = true;
	mu_uint32 WindowWidth = 1366;
	mu_uint32 WindowHeight = 768;

	mu_boolean EnableShadows = false;
	NShadowMode ShadowMode = NShadowMode::PCF;
	mu_float ShadowFarZ = 5000.0f;
	mu_boolean ShadowRightHanded = false;
	mu_uint32 ShadowCascadesCount = 4u;
	mu_uint32 ShadowResolution = 2048u;
	mu_float ShadowPartitioning = 0.95f;
	mu_uint32 ShadowFilterSize = 2u;
	mu_boolean ShadowFilterAcrossCascades = false;
	mu_boolean ShadowBestCascadeSearch = false;

	mu_boolean Antialiasing = false;
	mu_boolean VerticalSync = false;

	mu_float MusicVolume = 1.0f;
	mu_float SoundVolume = 1.0f;

	const mu_boolean Initialize()
	{
		SDL_RWops* fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eUser>(&fp, "config/game.json", "rb") == false)
		{
			return true;
		}

		mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
		std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
		SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
		SDL_RWclose(fp);

		const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
		jsonBuffer.reset();
		auto document = nlohmann::json::parse(inputBuffer.c_str());
		if (document.is_discarded() == true)
		{
			return true;
		}
		
#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_DESKTOP
		if (document.contains("WindowWidth") == true)
		{
			WindowWidth = document["WindowWidth"].get<mu_int32>();
		}

		if (document.contains("WindowHeight") == true)
		{
			WindowHeight = document["WindowHeight"].get<mu_int32>();
		}

		if (WindowWidth < 640 ||
			WindowHeight < 480)
		{
			WindowWidth = 640;
			WindowHeight = 480;
		}
#endif

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_DESKTOP
		if (document.contains("WindowMode") == true)
		{
			WindowMode = document["WindowMode"].get<mu_boolean>();
		}

		if (WindowMode == false)
		{
			SDL_DisplayMode dm = {};
			if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
			{
				WindowMode = true;
			}
			else
			{
				WindowWidth = dm.w;
				WindowHeight = dm.h;
			}
		}
#endif

		if (document.contains("Shadows") == true)
		{
			EnableShadows = document["Shadows"].get<mu_boolean>();
		}

		if (document.contains("ShadowMode") == true)
		{
			ShadowMode = static_cast<NShadowMode>(std::clamp(document["ShadowMode"].get<mu_uint32>(), MinShadowMode, MaxShadowMode));
		}

		if (document.contains("ShadowFarZ") == true)
		{
			ShadowFarZ = glm::clamp(document["ShadowFarZ"].get<mu_float>(), 1000.0f, 50000.0f);
		}

		if (document.contains("ShadowRightHanded") == true)
		{
			ShadowRightHanded = document["ShadowRightHanded"].get<mu_boolean>();
		}

		if (document.contains("ShadowCascadesCount") == true)
		{
			ShadowCascadesCount = glm::clamp(document["ShadowCascadesCount"].get<mu_uint32>(), 1u, static_cast<mu_uint32>(MAX_CASCADES));
		}

		if (document.contains("ShadowResolution") == true)
		{
			ShadowResolution = glm::clamp(document["ShadowResolution"].get<mu_uint32>(), 1024u, 8192u);
		}

		if (document.contains("ShadowPartitioning") == true)
		{
			ShadowPartitioning = glm::clamp(document["ShadowPartitioning"].get<mu_float>(), 0.0f, 1.0f);
		}

		if (document.contains("ShadowFilterSize") == true)
		{
			ShadowFilterSize = glm::clamp(document["ShadowFilterSize"].get<mu_uint32>(), 1u, 4u);
		}

		if (document.contains("ShadowFilterAcrossCascades") == true)
		{
			ShadowFilterAcrossCascades = document["ShadowFilterAcrossCascades"].get<mu_boolean>();
		}

		if (document.contains("ShadowBestCascadeSearch") == true)
		{
			ShadowBestCascadeSearch = document["ShadowBestCascadeSearch"].get<mu_boolean>();
		}

		if (document.contains("Antialiasing") == true)
		{
			Antialiasing = document["Antialiasing"].get<mu_boolean>();
		}

		if (document.contains("VerticalSync") == true)
		{
			VerticalSync = document["VerticalSync"].get<mu_boolean>();
		}

		if (document.contains("MusicVolume") == true)
		{
			MusicVolume = document["MusicVolume"].get<mu_float>();
		}

		if (document.contains("SoundVolume") == true)
		{
			SoundVolume = document["SoundVolume"].get<mu_float>();
		}

		return true;
	}

	void Destroy()
	{

	}

	void Save()
	{

	}

	const mu_boolean GetWindowMode()
	{
		return WindowMode;
	}

	const mu_uint32 GetWindowWidth()
	{
		return WindowWidth;
	}

	const mu_uint32 GetWindowHeight()
	{
		return WindowHeight;
	}

	const mu_boolean GetEnableShadows()
	{
		return EnableShadows;
	}

	const NShadowMode GetShadowMode()
	{
		return ShadowMode;
	}

	const mu_float GetShadowFarZ()
	{
		return ShadowFarZ;
	}

	const mu_boolean GetShadowRightHanded()
	{
		return ShadowRightHanded;
	}

	const mu_uint32 GetShadowCascadesCount()
	{
		return ShadowCascadesCount;
	}

	const mu_uint32 GetShadowResolution()
	{
		return ShadowResolution;
	}

	const mu_float GetShadowPartitioning()
	{
		return ShadowPartitioning;
	}

	const mu_uint32 GetShadowFilterSize()
	{
		return ShadowFilterSize;
	}

	const mu_boolean GetShadowFilterAcrossCascades()
	{
		return ShadowFilterAcrossCascades;
	}

	const mu_boolean GetShadowBestCascadeSearch()
	{
		return ShadowBestCascadeSearch;
	}

	const mu_boolean GetAntialiasing()
	{
		return Antialiasing;
	}

	const mu_boolean GetVerticalSync()
	{
		return VerticalSync;
	}
};