#include "stdafx.h"
#include "mu_environment.h"
#include "mu_resourcesmanager.h"
#include "mu_crypt.h"
#include "shared_binaryreader.h"
#include "mu_graphics.h"

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
	terrain->TerrainProgram = MUResourcesManager::GetProgram(programId);
	if (terrain->TerrainProgram == NInvalidShader)
	{
		mu_error("terrain program not found ({}, {})", filename, programId);
		return false;
	}

	terrain->TerrainShadowProgram = MUResourcesManager::GetProgram(programId + "_shadow");
	if (terrain->TerrainShadowProgram == NInvalidShader)
	{
		mu_error("terrain program not found ({}, {})", filename, programId);
		return false;
	}

	const auto grassProgramId = document["grass_program"].get<mu_utf8string>();
	terrain->GrassProgram = MUResourcesManager::GetProgram(grassProgramId);
	if (terrain->GrassProgram == NInvalidShader)
	{
		mu_error("terrain grass program not found ({}, {})", filename, grassProgramId);
		return false;
	}

	terrain->GrassShadowProgram = MUResourcesManager::GetProgram(grassProgramId + "_shadow");
	if (terrain->GrassShadowProgram == NInvalidShader)
	{
		mu_error("terrain grass program not found ({}, {})", filename, grassProgramId);
		return false;
	}

	terrain->HeightMultiplier = document["height_multiplier"].get<mu_float>();

	std::vector<Diligent::StateTransitionDesc> barriers;
	const auto heightmap = document["heightmap"].get<mu_utf8string>();
	if (terrain->LoadHeightmap(path + heightmap, barriers) == false)
	{
		mu_error("failed to load heightmap ({})", path + heightmap);
		return false;
	}

	if (terrain->GenerateNormal(barriers) == false)
	{
		mu_error("failed to generate normal ({})", filename);
		return false;
	}

	const auto navMesh = document["nav_mesh"].get<mu_utf8string>();
	if (terrain->LoadNavMesh(path + navMesh) == false)
	{
		return false;
	}

	const auto light = document["light"];
	if (light.is_array() == false || light.size() != 3)
	{
		mu_error("invalid terrain light ({})", path);
		return false;
	}

	for (mu_uint32 n = 0; n < 3; ++n)
	{
		terrain->Light[n] = light[n].get<mu_float>();
	}

	const auto lightmap = document["lightmap"].get<mu_utf8string>();
	if (terrain->LoadLightmap(path + lightmap, barriers) == false)
	{
		mu_error("failed to load lightmap ({})", path + lightmap);
		return false;
	}

	if (document.contains("light_position"))
	{
		const auto lightPosition = document["light_position"];
		if (lightPosition.is_array() == false || lightPosition.size() != 3)
		{
			mu_error("invalid terrain light position ({})", path);
			return false;
		}

		for (mu_uint32 n = 0; n < 3; ++n)
		{
			terrain->LightPosition[n] = lightPosition[n].get<mu_float>();
		}
	}

	if (terrain->GenerateBuffers(barriers) == false)
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
	if (terrain->LoadTextures(path, textures, filter, wrap, uvNormal, uvScaled, texturesMap, barriers) == false)
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

		if (terrain->LoadGrassTextures(path, grassTextures, filter, wrap, grassTexturesMap, barriers) == false)
		{
			mu_error("failed to load textures ({})", filename);
			return false;
		}
	}

	const auto mappings = document["mappings"].get<mu_utf8string>();
	if (terrain->LoadMappings(path + mappings, texturesMap, grassTexturesMap, barriers) == false)
	{
		mu_error("failed to load mappings ({})", path + mappings);
		return false;
	}

	const auto attributes = document["attributes"].get<mu_utf8string>();
	if (terrain->LoadAttributes(path + attributes, barriers) == false)
	{
		mu_error("failed to load attributes ({})", path + attributes);
		return false;
	}

	if (terrain->PrepareSettings(path, document, barriers) == false)
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
			const mu_utf8string modelPath = model["path"].get<mu_utf8string>();

			NModelPtr model = std::make_unique<NModel>();
			if (
				model->Load(
					fmt::format("{}_{}", terrain->GetId(), id),
					path + modelPath
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

	const auto immediateContext = MUGraphics::GetImmediateContext();
	immediateContext->TransitionResourceStates(static_cast<mu_uint32>(barriers.size()), barriers.data());

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
	std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
	SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
	jsonBuffer.reset();
	auto document = nlohmann::json::parse(inputBuffer.c_str());
	if (document.is_discarded() == true)
	{
		mu_error("objects.json malformed ({})", filename);
		return false;
	}

	const auto &objects = document["objects"];
	for (const auto &jobject : objects)
	{
		TObject::Settings object;
		object.Type = jobject["type"].get<mu_uint16>();
		object.Renderable = jobject["renderable"].get<mu_boolean>();
		object.Interactive = jobject["interactive"].get<mu_boolean>();
		object.LightEnable = jobject["light_enable"].get<mu_boolean>();
		object.ShouldFade = false;
		if (jobject.contains("should_fade"))
			object.ShouldFade = jobject["should_fade"].get<mu_boolean>();

		const auto &jlight = jobject["light"];
		object.Light.Mode = LightModeFromString(jlight["mode"].get<mu_utf8string>());
		switch (object.Light.Mode)
		{
		case EntityLightMode::Terrain:
			{
				object.Light.PrimaryLight = jlight["primary_light"].get<mu_boolean>();
				object.Light.LightIntensity = jlight["intensity"].get<mu_float>();

				const auto &jcolor = jlight["color"];
				object.Light.Color[0] = jcolor[0].get<mu_float>();
				object.Light.Color[1] = jcolor[1].get<mu_float>();
				object.Light.Color[2] = jcolor[2].get<mu_float>();
			}
			break;

		case EntityLightMode::Fixed:
			{
				const auto &jcolor = jlight["color"];
				object.Light.Color[0] = jcolor[0].get<mu_float>();
				object.Light.Color[1] = jcolor[1].get<mu_float>();
				object.Light.Color[2] = jcolor[2].get<mu_float>();
			}
			break;

		case EntityLightMode::SinWorldTime:
			{
				object.Light.TimeMultiplier = jlight["time"].get<mu_float>();
				object.Light.LightMultiplier = jlight["multiplier"].get<mu_float>();
				object.Light.LightAdd = jlight["add"].get<mu_float>();
			}
			break;
		}

		const auto &jposition = jobject["position"];
		object.Position[0] = jposition[0].get<mu_float>();
		object.Position[1] = jposition[1].get<mu_float>();
		object.Position[2] = jposition[2].get<mu_float>();

		const auto &jangle = jobject["angle"];
		object.Angle[0] = jangle[0].get<mu_float>();
		object.Angle[1] = jangle[1].get<mu_float>();
		object.Angle[2] = jangle[2].get<mu_float>();

		object.Scale = jobject["scale"].get<mu_float>();

		const auto &jbbox = jobject["bounding_box"];
		const auto &jbboxMin = jbbox["min"];
		object.BBoxMin[0] = jbboxMin[0].get<mu_float>();
		object.BBoxMin[1] = jbboxMin[1].get<mu_float>();
		object.BBoxMin[2] = jbboxMin[2].get<mu_float>();

		const auto &jbboxMax = jbbox["max"];
		object.BBoxMax[0] = jbboxMax[0].get<mu_float>();
		object.BBoxMax[1] = jbboxMax[1].get<mu_float>();
		object.BBoxMax[2] = jbboxMax[2].get<mu_float>();

		auto modelIter = models.find(object.Type);
		if (modelIter == models.end())
		{
			object.Model = nullptr;
			object.Renderable = false;
		}
		else
		{
			object.Model = modelIter->second;
		}

		Objects->Add(
			object
		);
	}

	return true;
}