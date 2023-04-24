#include "stdafx.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletonmanager.h"
#include "mu_graphics.h"
#include "mu_model.h"
#include "mu_textures.h"
#include "res_renders.h"
#include "res_items.h"
#include <ShaderMacroHelper.hpp>

template<const EGameDirectoryType dirType>
NEXTMU_INLINE std::vector<mu_char> mu_readshader(mu_utf8string filename)
{
	NormalizePath(filename);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		return std::vector<mu_char>();
	}

	std::vector<mu_char> data(SDL_RWsize(fp) + 1);
	SDL_RWread(fp, data.data(), data.size() - 1, 1);
	SDL_RWclose(fp);
	data[data.size() - 1] = '\0';

	return data;
}

typedef std::unique_ptr<NModel> ModelPointer;
typedef std::unique_ptr<NGraphicsTexture> TexturePointer;

namespace MUResourcesManager
{
	std::map<mu_utf8string, mu_shader> Programs;
	std::map<mu_utf8string, TexturePointer> Textures;
	std::map<mu_utf8string, ModelPointer> Models;

	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, const mu_utf8string resourceId);
	const mu_boolean LoadPrograms(const mu_utf8string basePath, const nlohmann::json &programs);
	const mu_boolean LoadTextures(const mu_utf8string basePath, const nlohmann::json &textures);
	const mu_boolean LoadModels(const mu_utf8string basePath, const nlohmann::json &models);

	const mu_boolean Load()
	{
		const mu_utf8string path = "data/";
		const mu_utf8string filename = path + "resources.json";

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
			mu_error("resources malformed ({})", filename);
			return false;
		}

		if (document.contains("shaders"))
		{
			const auto shaders = document["shaders"];
			if (LoadPrograms(path, shaders) == false)
			{
				return false;
			}
		}

		if (document.contains("textures"))
		{
			const auto textures = document["textures"];
			if (LoadTextures(path, textures) == false)
			{
				return false;
			}
		}

		if (document.contains("models"))
		{
			const auto models = document["models"];
			if (LoadModels(path, models) == false)
			{
				return false;
			}
		}

		return true;
	}

	void Destroy()
	{
		Programs.clear();
		Textures.clear();
		Models.clear();
	}

	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, const mu_utf8string resourceId)
	{
		const auto device = MUGraphics::GetDevice();
		const mu_utf8string folder = MUGraphics::GetShaderFolder();
		const mu_utf8string ext = MUGraphics::GetShaderExtension();

		const auto vertexBuffer = mu_readshader<EGameDirectoryType::eSupport>(vertex + folder + "shader.vs" + ext);
		const auto fragmentBuffer = mu_readshader<EGameDirectoryType::eSupport>(fragment + folder + "shader.fs" + ext);
		if (vertexBuffer.empty() || fragmentBuffer.empty())
		{
			return false;
		}

		Diligent::ShaderMacroHelper macros;
		macros.AddShaderMacro("SKELETON_TEXTURE_WIDTH", MUSkeletonManager::BonesTextureWidth);
		macros.AddShaderMacro("SKELETON_TEXTURE_HEIGHT", MUSkeletonManager::BonesTextureHeight);

		Diligent::ShaderCreateInfo createInfo;
		createInfo.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		createInfo.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
		createInfo.Desc.UseCombinedTextureSamplers = true;
		createInfo.Source = vertexBuffer.data();
		createInfo.Macros = macros;

		Diligent::RefCntAutoPtr<Diligent::IShader> vertexShader;
		device->CreateShader(createInfo, &vertexShader);
		if (vertexShader == nullptr)
		{
			return false;
		}

		createInfo.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		createInfo.Source = fragmentBuffer.data();

		Diligent::RefCntAutoPtr<Diligent::IShader> pixelShader;
		device->CreateShader(createInfo, &pixelShader);
		if (pixelShader == nullptr)
		{
			return false;
		}

		NCombinedShader shader;
		shader.Layout = GetInputLayout(resourceId);
		shader.Resource = GetPipelineResource(resourceId);
		shader.Vertex = vertexShader;
		shader.Pixel = pixelShader;
		const auto index = RegisterShader(shader);

		Programs.insert(std::pair(id, index));

		return true;
	}

	const mu_boolean LoadPrograms(const mu_utf8string basePath, const nlohmann::json &programs)
	{
		for (const auto &p : programs)
		{
			const mu_utf8string id = p["id"];
			const mu_utf8string vertex = p["vertex"];
			const mu_utf8string fragment = p["fragment"];
			const mu_utf8string resourceId = p["resource_id"];

			if (LoadProgram(id, basePath + vertex, basePath + fragment, resourceId) == false)
			{
				return false;
			}
		}

		return true;
	}

	const mu_boolean LoadTextures(const mu_utf8string basePath, const nlohmann::json &textures)
	{
		for (const auto &t : textures)
		{
			const mu_utf8string id = t["id"];
			const mu_utf8string path = t["path"];
			mu_utf8string filter = "linear";
			mu_utf8string wrap = "repeat";

			if (t.contains("filter"))
			{
				filter = t["filter"].get<mu_utf8string>();
			}

			if (t.contains("wrap"))
			{
				wrap = t["wrap"].get<mu_utf8string>();
			}

			auto texture = MUTextures::Load(basePath + path, MUTextures::CalculateSamplerFlags(filter, wrap));
			if (!texture)
			{
				mu_error("failed to load texture ({})", path);
				return false;
			}

			Textures.insert(std::pair(id, std::move(texture)));
		}

		return true;
	}

	const mu_boolean LoadModels(const mu_utf8string basePath, const nlohmann::json &models)
	{
		for (const auto &m : models)
		{
			const mu_utf8string id = m["id"];
			const mu_utf8string path = m["path"];

			std::unique_ptr<NModel> model(new_nothrow NModel());
			if (model->Load(id, basePath + path) == false)
			{
				mu_error("failed to load model ({})", path);
				return false;
			}

			Models.insert(std::pair(id, std::move(model)));
		}

		return true;
	}

	const mu_shader GetProgram(const mu_utf8string id)
	{
		auto iter = Programs.find(id);
		if (iter == Programs.end()) return NInvalidShader;
		return iter->second;
	}

	NGraphicsTexture *GetTexture(const mu_utf8string id)
	{
		auto iter = Textures.find(id);
		if (iter == Textures.end()) return nullptr;
		return iter->second.get();
	}

	NModel *GetModel(const mu_utf8string id)
	{
		auto iter = Models.find(id);
		if (iter == Models.end()) return nullptr;
		return iter->second.get();
	}
};