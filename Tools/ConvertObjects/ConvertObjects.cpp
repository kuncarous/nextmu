// ConvertObjects.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "shared_binaryreader.h"

#include <iostream>

NEXTMU_INLINE void XorDecrypt(mu_uint8 *dest, const mu_uint8 *src, mu_uint32 size)
{
	static const std::array<mu_uint8, 16> XorKey = { { 0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27, 0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2 } };
	mu_uint16 key = 0x5E;
	for (mu_uint32 index = 0; index < size; ++index, ++src, ++dest)
	{
		const mu_uint8 s = *src;
		*dest = (s ^ XorKey[index % XorKey.size()]) - static_cast<mu_uint8>(key);
		key = static_cast<mu_uint8>(static_cast<mu_uint32>(s) + 0x3D);
	}
}

enum class LightMode
{
	Terrain,
	Fixed,
	SinWorldTime,
};

const LightMode LightModeFromString(const mu_utf8string mode)
{
	if (mode == "terrain") return LightMode::Terrain;
	if (mode == "fixed") return LightMode::Fixed;
	if (mode == "sinworld") return LightMode::SinWorldTime;
	return LightMode::Terrain;
}

const mu_utf8string LightModeToString(const LightMode mode)
{
	switch (mode)
	{
	default:
	case LightMode::Terrain: return "terrain";
	case LightMode::Fixed: return "fixed";
	case LightMode::SinWorldTime: return "sinworld";
	}
}

struct LightObjectSetting
{
	mu_boolean Forced = false;
	LightMode Mode = LightMode::Terrain;
	mu_boolean PrimaryLight = true;
	glm::vec3 Color = glm::vec3(0.0f, 0.0f, 0.0f);
	mu_float LightIntensity = 1.0f; // LightEnable ? 1.0f : 0.1f
	// func(WorldTime * TimeMultiplier) * LightMultiplier + LightAdd
	// func : sinf, cosf, tanf, ...
	mu_float TimeMultiplier = 0.003f;
	mu_float LightMultiplier = 0.5f;
	mu_float LightAdd = 0.5f;
};

struct ObjectSetting
{
	mu_boolean Renderable = true;
	mu_boolean Interactive = false;
	mu_boolean LightEnable = true;
	LightObjectSetting Light;
	glm::vec3 BBoxMin = glm::vec3(-40.0f, -40.0f, 0.0f);
	glm::vec3 BBoxMax = glm::vec3(40.0f, 40.0f, 80.0f);
};

ObjectSetting DefaultSettings;
std::map<mu_uint32, ObjectSetting> Settings;

void OpenObjectsSettings(const mu_utf8string filename)
{
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		std::cout << "settings configuration missing, using default" << std::endl;
		return;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_char[]> buffer(new_nothrow mu_char[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	const mu_utf8string inputBuffer = JsonStripComments(buffer.get(), static_cast<mu_uint32>(fileLength));
	buffer.reset();

	auto document = nlohmann::json::parse(inputBuffer.c_str());
	if (document.is_discarded() == true)
	{
		return;
	}

	if (!document.contains("objects"))
	{
		return;
	}

	const auto &objects = document["objects"];
	for (const auto &object : objects)
	{
		const mu_uint32 type = object["type"].get<mu_uint32>();

		ObjectSetting settings;

		if (object.contains("renderable"))
		{
			settings.Renderable = object["renderable"].get<mu_boolean>();
		}

		if (object.contains("light_enable"))
		{
			settings.LightEnable = object["light_enable"].get<mu_boolean>();
		}

		if (object.contains("light"))
		{
			const auto &light = object["light"];
			const auto mode = LightModeFromString(light["mode"].get<mu_utf8string>());
			settings.Light.Forced = true;
			settings.Light.Mode = mode;

			switch (mode)
			{
			case LightMode::Terrain:
			case LightMode::Fixed:
				{
					const auto &color = light["color"];
					settings.Light.Color[0] = color[0].get<mu_float>();
					settings.Light.Color[1] = color[1].get<mu_float>();
					settings.Light.Color[2] = color[2].get<mu_float>();
				}
				break;
			}

			switch (mode)
			{
			case LightMode::Terrain:
				{
					if (light.contains("primary_light"))
					{
						settings.Light.PrimaryLight = light["primary_light"].get<mu_boolean>();
					}
					settings.Light.LightIntensity = light["intensity"].get<mu_float>();
				}
				break;

			case LightMode::SinWorldTime:
				{
					settings.Light.TimeMultiplier = light["time"].get<mu_float>();
					settings.Light.LightMultiplier = light["multiplier"].get<mu_float>();
					settings.Light.LightAdd = light["add"].get<mu_float>();
				}
				break;
			}
		}

		if (object.contains("bbox"))
		{
			const auto &bbox = object["bounding_box"];
			const auto &bboxMin = bbox["min"];
			settings.BBoxMin[0] = bboxMin[0].get<mu_float>();
			settings.BBoxMin[1] = bboxMin[1].get<mu_float>();
			settings.BBoxMin[2] = bboxMin[2].get<mu_float>();

			const auto &bboxMax = bbox["max"];
			settings.BBoxMax[0] = bboxMax[0].get<mu_float>();
			settings.BBoxMax[1] = bboxMax[1].get<mu_float>();
			settings.BBoxMax[2] = bboxMax[2].get<mu_float>();
		}

		Settings.insert(std::pair(type, settings));
	}
}

void ConvertObjects(const mu_utf8string filename)
{
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		std::cout << "encterrain.obj missing" << std::endl;
		return;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);
	fp = nullptr;

	XorDecrypt(buffer.get(), buffer.get(), static_cast<mu_uint32>(fileLength));

	NBinaryReader reader(buffer.get(), static_cast<mu_uint32>(fileLength));
	mu_uint8 version = reader.Read<mu_uint8>();
	mu_uint32 mapNumber = static_cast<mu_uint32>(reader.Read<mu_uint8>());
	mu_uint32 objectsCount = static_cast<mu_uint32>(reader.Read<mu_uint16>());

	nlohmann::ordered_json jobjects = nlohmann::ordered_json::array();
	for (mu_uint32 n = 0; n < objectsCount; ++n)
	{
		glm::vec3 position, angle;
		mu_float scale;

		mu_uint16 type = reader.Read<mu_uint16>();

		auto settingsIter = Settings.find(type);
		ObjectSetting settings = (
			settingsIter != Settings.end()
			? settingsIter->second
			: DefaultSettings
		);

		position[0] = reader.Read<mu_float>();
		position[1] = reader.Read<mu_float>();
		position[2] = reader.Read<mu_float>();

		angle[0] = reader.Read<mu_float>();
		angle[1] = reader.Read<mu_float>();
		angle[2] = reader.Read<mu_float>();

		scale = reader.Read<mu_float>();

		mu_boolean useLight = settings.LightEnable, fullLight = true, primaryLight = settings.Light.PrimaryLight;
		if (version > 0)
		{
			useLight = reader.Read<mu_boolean>();
			fullLight = reader.Read<mu_boolean>();
		}

		if (version >= 2)
		{
			primaryLight = reader.Read<mu_boolean>();
		}

		glm::vec3 color = settings.Light.Color;
		if (version >= 3)
		{
			color[0] = reader.Read<mu_float>();
			color[1] = reader.Read<mu_float>();
			color[2] = reader.Read<mu_float>();
		}

		nlohmann::ordered_json jobject;

		jobject["type"] = type;
		jobject["renderable"] = settings.Renderable;
		jobject["interactive"] = settings.Interactive;
		jobject["light_enable"] = useLight;

		nlohmann::ordered_json jposition = nlohmann::ordered_json::array();
		jposition.push_back(position[0]);
		jposition.push_back(position[1]);
		jposition.push_back(position[2]);
		jobject["position"] = jposition;

		nlohmann::ordered_json jangle = nlohmann::ordered_json::array();
		jangle.push_back(angle[0]);
		jangle.push_back(angle[1]);
		jangle.push_back(angle[2]);
		jobject["angle"] = jangle;

		jobject["scale"] = scale;

		if (version > 0 && settings.Light.Forced == false)
		{
			nlohmann::ordered_json jlight;
			jlight["mode"] = LightModeToString(LightMode::Terrain);
			jlight["primary_light"] = primaryLight;
			jlight["intensity"] = (
				useLight && fullLight
				? 1.0f
				: 0.1f
			);

			nlohmann::ordered_json jcolor = nlohmann::ordered_json::array();
			jcolor.push_back(color[0]);
			jcolor.push_back(color[1]);
			jcolor.push_back(color[2]);
			jlight["color"] = jcolor;
		}
		else
		{
			nlohmann::ordered_json jlight;
			jlight["mode"] = LightModeToString(settings.Light.Mode);

			switch (settings.Light.Mode)
			{
			case LightMode::Terrain:
				{
					jlight["primary_light"] = primaryLight;
					jlight["intensity"] = settings.Light.LightIntensity;
				}
				break;

			case LightMode::SinWorldTime:
				{
					jlight["time"] = settings.Light.TimeMultiplier;
					jlight["multiplier"] = settings.Light.LightMultiplier;
					jlight["add"] = settings.Light.LightAdd;
				}
				break;
			}

			switch (settings.Light.Mode)
			{
			case LightMode::Terrain:
			case LightMode::Fixed:
				{
					nlohmann::ordered_json jcolor = nlohmann::ordered_json::array();
					jcolor.push_back(color[0]);
					jcolor.push_back(color[1]);
					jcolor.push_back(color[2]);
					jlight["color"] = jcolor;
				}
				break;
			}

			jobject["light"] = jlight;
		}

		nlohmann::ordered_json bboxMin = nlohmann::ordered_json::array();
		bboxMin.push_back(settings.BBoxMin[0]);
		bboxMin.push_back(settings.BBoxMin[1]);
		bboxMin.push_back(settings.BBoxMin[2]);

		nlohmann::ordered_json bboxMax = nlohmann::ordered_json::array();
		bboxMax.push_back(settings.BBoxMax[0]);
		bboxMax.push_back(settings.BBoxMax[1]);
		bboxMax.push_back(settings.BBoxMax[2]);

		nlohmann::ordered_json bbox;
		bbox["min"] = bboxMin;
		bbox["max"] = bboxMax;
		jobject["bounding_box"] = bbox;

		jobjects.push_back(jobject);
	}

	nlohmann::ordered_json jdocument;
	jdocument["objects"] = jobjects;

	auto dotPos = filename.find_last_of('.');
	const mu_utf8string outfilename = (
		(
			dotPos == mu_utf8string::npos
			? filename
			: filename.substr(0, dotPos)
		) + ".objects.json"
	);

	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, outfilename, "wb") == false)
	{
		std::cout << "failed to save file" << std::endl;
		return;
	}

	const auto output = jdocument.dump(1, '\t');
	SDL_RWwrite(fp, output.c_str(), output.size(), 1);
	SDL_RWclose(fp);
}

int main(int argc, char *argv[])
{
	if (SDL_Init(0) < 0)
	{
		std::cout << "failed to initialize SDL" << std::endl;
		return 0;
	}

    if (argc != 2)
    {
        std::cout << "encterrain.obj missing" << std::endl;
        return 0;
    }

	const mu_utf8string filename = argv[1];
	auto dotPos = filename.find_last_of('.');
	const mu_utf8string settingsfile = (
		(
			dotPos == mu_utf8string::npos
			? filename
			: filename.substr(0, dotPos)
		) + ".settings.json"
	);
	OpenObjectsSettings(settingsfile);
	ConvertObjects(filename);

	SDL_Quit();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
