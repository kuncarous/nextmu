#include "mu_precompiled.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletonmanager.h"
#include "mu_textureattachments.h"
#include "mu_config.h"
#include "mu_graphics.h"
#include "mu_model.h"
#include "mu_textures.h"
#include "res_renders.h"
#include "res_items.h"
#include <boost/algorithm/string/replace.hpp>

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
			mu_error("resources.json missing ({})", filename);
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

		if (document.contains("attachments"))
		{
			const auto jattachments = document["attachments"];
			for (const auto &jattachment : jattachments)
			{
				MUTextureAttachments::CreateAttachmentTypeByString(jattachment.get<mu_utf8string>());
			}
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
		MUGraphics::FlushContext();

		if (document.contains("models"))
		{
			const auto models = document["models"];
			if (LoadModels(path, models) == false)
			{
				return false;
			}
		}

		MUGraphics::FlushContext();

		return true;
	}

	void Destroy()
	{
		Programs.clear();
		Textures.clear();
		Models.clear();
	}

	const mu_utf8string FixShaderSourceByDeviceType(const Diligent::RENDER_DEVICE_TYPE deviceType, mu_utf8string source)
	{
		switch (deviceType)
		{
		case Diligent::RENDER_DEVICE_TYPE_GL:
		case Diligent::RENDER_DEVICE_TYPE_GLES:
			{
				boost::replace_all(source, "half4", "float4");
				boost::replace_all(source, "half3", "float3");
				boost::replace_all(source, "half2", "float2");
				boost::replace_all(source, "half", "float");
			}
		default: return source;
		}
	}

	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, NShaderSettings &settings)
	{
		const auto device = MUGraphics::GetDevice();

		const auto vertexBuffer = mu_readshader<EGameDirectoryType::eSupport>(vertex);
		const auto fragmentBuffer = mu_readshader<EGameDirectoryType::eSupport>(fragment);
		if (vertexBuffer.empty() || fragmentBuffer.empty())
		{
			return false;
		}

		const auto deviceType = MUGraphics::GetDeviceType();
		const mu_utf8string vertexBufferStr = FixShaderSourceByDeviceType(deviceType, settings.AppendVertex + mu_utf8string(vertexBuffer.begin(), vertexBuffer.end()));
		const mu_utf8string fragmentBufferStr = FixShaderSourceByDeviceType(deviceType, settings.AppendPixel + mu_utf8string(fragmentBuffer.begin(), fragmentBuffer.end()));

		Diligent::ShaderCreateInfo createInfo;
#ifndef NDEBUG
		createInfo.Desc.Name = id.c_str();
#endif
		createInfo.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		createInfo.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
		createInfo.Desc.UseCombinedTextureSamplers = true;
		createInfo.Source = vertexBufferStr.c_str();
		createInfo.Macros = settings.VertexMacros;

		Diligent::RefCntAutoPtr<Diligent::IShader> vertexShader;
		device->CreateShader(createInfo, &vertexShader);
		if (vertexShader == nullptr)
		{
			return false;
		}

		createInfo.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		createInfo.Source = fragmentBufferStr.c_str();
		createInfo.Macros = settings.PixelMacros;

		Diligent::RefCntAutoPtr<Diligent::IShader> pixelShader;
		device->CreateShader(createInfo, &pixelShader);
		if (pixelShader == nullptr)
		{
			return false;
		}

		NCombinedShader shader;
		shader.Layout = settings.InputLayout;
		shader.Vertex = vertexShader;
		shader.Pixel = pixelShader;
		shader.ResourceSignatures = std::move(settings.ResourceSignatures);
		shader.Resource = settings.Resource;
		const auto index = RegisterShader(shader);

		Programs.insert(std::pair(id, index));

		return true;
	}

	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, const mu_utf8string resourceId, const nlohmann::json &jmacros)
	{
		Diligent::ShaderMacroHelper macros;
		macros.AddShaderMacro("SKELETON_TEXTURE_WIDTH", MUSkeletonManager::BonesTextureWidth);
		macros.AddShaderMacro("SKELETON_TEXTURE_HEIGHT", MUSkeletonManager::BonesTextureHeight);
		macros.AddShaderMacro("USE_SHADOW", MUConfig::GetEnableShadows() ? 1 : 0);
		macros.AddShaderMacro("SHADOW_MODE", static_cast<mu_int32>(MUConfig::GetShadowMode()));
		macros.AddShaderMacro("SHADOW_FILTER_SIZE", static_cast<mu_int32>(MUConfig::GetShadowFilterSize()));
		macros.AddShaderMacro("FILTER_ACROSS_CASCADES", static_cast<mu_int32>(MUConfig::GetShadowFilterAcrossCascades()));
		macros.AddShaderMacro("BEST_CASCADE_SEARCH", static_cast<mu_int32>(MUConfig::GetShadowBestCascadeSearch()));
#if 0
		macros.AddShaderMacro("DSHADOW_COLOR", 1);
#endif

		for (const auto jmacro : jmacros)
		{
			const auto type = jmacro["type"].get<mu_utf8string>();
			const auto name = jmacro["name"].get<mu_utf8string>();

			if (type == "integer")
			{
				macros.AddShaderMacro(name.c_str(), jmacro["value"].get<mu_int32>());
			}
			else if (type == "float")
			{
				macros.AddShaderMacro(name.c_str(), jmacro["value"].get<mu_float>());
			}
			else if (type == "boolean")
			{
				macros.AddShaderMacro(name.c_str(), jmacro["value"].get<mu_boolean>());
			}
			else if (type == "string")
			{
				macros.AddShaderMacro(name.c_str(), jmacro["value"].get<mu_utf8string>().c_str());
			}
		}

		NShaderSettings settings;
		settings.VertexMacros = macros;
		settings.PixelMacros = macros;
		settings.InputLayout = GetInputLayout(resourceId);
		settings.Resource = GetPipelineResource(resourceId);

		return LoadProgram(id, vertex, fragment, settings);
	}

	const mu_boolean LoadPrograms(const mu_utf8string basePath, const nlohmann::json &programs)
	{
		const auto dummyArray = nlohmann::json().array();
		for (const auto &p : programs)
		{
			const mu_utf8string id = p["id"].get<mu_utf8string>();
			const mu_utf8string vertex = p["vertex"].get<mu_utf8string>();
			const mu_utf8string fragment = p["fragment"].get<mu_utf8string>();
			const mu_utf8string resourceId = p["resource_id"].get<mu_utf8string>();
			const auto &macros = p.contains("macros") ? p["macros"] : dummyArray;

			if (LoadProgram(id, basePath + vertex, basePath + fragment, resourceId, macros) == false)
			{
				return false;
			}
		}

		return true;
	}

	const mu_boolean LoadTextures(const mu_utf8string basePath, const nlohmann::json &textures)
	{
		auto swapchain = MUGraphics::GetSwapChain();

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
		auto swapchain = MUGraphics::GetSwapChain();

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
