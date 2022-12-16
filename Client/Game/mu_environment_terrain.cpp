#include "stdafx.h"
#include "mu_environment.h"
#include "mu_resourcesmanager.h"
#include "mu_crypt.h"
#include "mu_binaryreader.h"

const mu_boolean NEnvironment::LoadTerrain(mu_utf8string path)
{
	/*
		Lets avoid circular dependencies, so instead of loading the terrain json in NTerrain we will allocate and configure to load from here.
	*/
	NormalizePath<true>(path);

	const mu_utf8string filename = path + "terrain.json";
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		mu_error("terrain.json missing ({})", filename);
		return false;
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
		mu_error("terrain.json malformed ({})", filename);
		return false;
	}

	auto terrain = std::make_unique<NTerrain>();
	terrain->Id = document["id"].get<mu_utf8string>();

	const auto programId = document["program"].get<mu_utf8string>();
	terrain->Program = MUResourcesManager::GetProgram(programId);
	if (bgfx::isValid(terrain->Program) == false)
	{
		mu_error("terrain program not found ({}, {})", filename, programId);
		return false;
	}

	const auto grassProgramId = document["grass_program"].get<mu_utf8string>();
	terrain->GrassProgram = MUResourcesManager::GetProgram(grassProgramId);
	if (bgfx::isValid(terrain->GrassProgram) == false)
	{
		mu_error("terrain grass program not found ({}, {})", filename, grassProgramId);
		return false;
	}

	terrain->HeightMultiplier = document["height_multiplier"].get<mu_float>();

	const auto heightmap = document["heightmap"].get<mu_utf8string>();
	if (terrain->LoadHeightmap(path + heightmap) == false)
	{
		mu_error("failed to load heightmap ({})", path + heightmap);
		return false;
	}

	if (terrain->GenerateNormal() == false)
	{
		mu_error("failed to generate normal ({})", filename);
		return false;
	}

	const auto lightmap = document["lightmap"].get<mu_utf8string>();
	if (terrain->LoadLightmap(path + lightmap) == false)
	{
		mu_error("failed to load lightmap ({})", path + lightmap);
		return false;
	}

	const auto light = document["light"];
	if (light.is_array() == false || light.size() != 3)
	{
		mu_error("invalid terrain light ({})", path + lightmap);
		return false;
	}

	for (mu_uint32 n = 0; n < 3; ++n)
	{
		terrain->Light[n] = light[n].get<mu_float>();
	}

	if (terrain->GenerateBuffers() == false)
	{
		return false;
	}

	const auto uv = document["uv"];
	if (uv.is_object() == false)
	{
		mu_error("terrain.json malformed ({})", filename);
		return false;
	}

	const auto uvNormal = uv["normal"].get<mu_float>();
	const auto uvScaled = uv["scaled"].get<mu_float>();

	const auto filter = document["filter"].get<mu_utf8string>();
	const auto wrap = document["wrap"].get<mu_utf8string>();

	const auto textures = document["textures"];
	if (textures.is_array() == false)
	{
		mu_error("terrain.json textures missing ({})", filename);
		return false;
	}

	std::map<mu_uint32, mu_uint32> texturesMap, grassTexturesMap;
	if (terrain->LoadTextures(path, textures, filter, wrap, uvNormal, uvScaled, texturesMap) == false)
	{
		mu_error("failed to load textures ({})", filename);
		return false;
	}

	if (document.contains("grass_textures"))
	{
		const auto grassTextures = document["grass_textures"];
		if (grassTextures.is_array() == false)
		{
			mu_error("terrain.json textures missing ({})", filename);
			return false;
		}

		if (terrain->LoadGrassTextures(path, grassTextures, filter, wrap, grassTexturesMap) == false)
		{
			mu_error("failed to load textures ({})", filename);
			return false;
		}
	}

	const auto mappings = document["mappings"].get<mu_utf8string>();
	if (terrain->LoadMappings(path + mappings, texturesMap, grassTexturesMap) == false)
	{
		mu_error("failed to load mappings ({})", path + mappings);
		return false;
	}

	const auto attributes = document["attributes"].get<mu_utf8string>();
	if (terrain->LoadAttributes(path + attributes) == false)
	{
		mu_error("failed to load attributes ({})", path + attributes);
		return false;
	}

	if (terrain->PrepareSettings(path, document) == false)
	{
		return false;
	}

	std::map<mu_uint32, NModel *> modelsMap;
	if (document.contains("models"))
	{
		const auto models = document["models"];
		for (const auto &model : models)
		{
			const mu_uint32 id = model["id"].get<mu_uint32>();
			const mu_utf8string path = model["path"].get<mu_utf8string>();

			NModelPtr model = std::make_unique<NModel>();
			if (
				model->Load(
					fmt::format("{}_{}", terrain->GetId(), id),
					path
				) == false
			)
			{
				mu_error("failed to load terrain model ({})", path);
				return false;
			}

			modelsMap.insert(std::pair(id, model.get()));
			Models.push_back(std::move(model));
		}
	}

	const auto objects = document["objects"].get<mu_utf8string>();
	if (LoadObjects(path + objects, modelsMap) == false)
	{
		mu_error("failed to load terrain objects ({})", objects);
		return false;
	}

	Terrain = std::move(terrain);

	return true;
}

const mu_boolean NEnvironment::LoadObjects(mu_utf8string filename, const std::map<mu_uint32, NModel *> models)
{
	/*
	* This will be replaced by objects.json, since we don't want to assign the light per object at runtime but instead get it from the resource file.
	*/
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		mu_error("objects file not found ({})", filename);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	XorDecrypt(buffer.get(), buffer.get(), static_cast<mu_uint32>(fileLength));

	NBinaryReader reader(buffer.get(), static_cast<mu_uint32>(fileLength));
	mu_uint8 version = reader.Read<mu_uint8>();
	mu_uint32 mapNumber = static_cast<mu_uint32>(reader.Read<mu_uint8>());
	mu_uint32 objectsCount = static_cast<mu_uint32>(reader.Read<mu_uint16>());

	for (mu_uint32 n = 0; n < objectsCount; ++n)
	{
		glm::vec3 position, angle;
		mu_float scale;

		mu_uint16 type = reader.Read<mu_uint16>();

		position[0] = reader.Read<mu_float>();
		position[1] = reader.Read<mu_float>();
		position[2] = reader.Read<mu_float>();

		angle[0] = reader.Read<mu_float>();
		angle[1] = reader.Read<mu_float>();
		angle[2] = reader.Read<mu_float>();

		scale = reader.Read<mu_float>();

		auto modelIter = models.find(type);
		if (modelIter == models.end()) continue;

		AddObject(
			modelIter->second,
			glm::vec3(1.0f, 1.0f, 1.0f),
			position,
			angle,
			scale
		);
	}

	return true;
}