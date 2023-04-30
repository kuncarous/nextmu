#include "stdafx.h"
#include "mu_config.h"

namespace MUConfig
{
	mu_boolean WindowMode = true;
	mu_uint32 WindowWidth = 1366;
	mu_uint32 WindowHeight = 768;

	mu_boolean EnableShadows = true;

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

	const mu_boolean GetAntialiasing()
	{
		return Antialiasing;
	}

	const mu_boolean GetVerticalSync()
	{
		return VerticalSync;
	}
};