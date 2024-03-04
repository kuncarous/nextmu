#include "mu_precompiled.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletonmanager.h"
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

const mu_boolean NResourcesManager::Load(const mu_utf8string filename)
{
	const auto pathIndex = filename.find_last_not_of('/');
	const auto path = pathIndex != mu_utf8string::npos ? filename.substr(0, pathIndex + 1) : "./";

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
			CreateAttachmentTypeByCRC32(GetAttachmentTypeFromId(jattachment.get<mu_utf8string>().c_str()));
		}
	}

	if (document.contains("shaders"))
	{
		const auto shaders = document["shaders"];
		LoadPrograms(path, shaders);
	}

	if (document.contains("textures"))
	{
		const auto textures = document["textures"];
		LoadTextures(path, textures);
	}
	MUGraphics::FlushContext();

	if (document.contains("models"))
	{
		const auto models = document["models"];
		LoadModels(path, models);
	}
	MUGraphics::FlushContext();

	ResourcesToLoad = ResourcesQueue.size();

	return true;
}

void NResourcesManager::Destroy()
{
	ProgramsLookup.clear();
	TexturesLookup.clear();
	ModelsLookup.clear();
	Shaders.clear();
	AttachmentsMap.clear();
}

const mu_boolean NResourcesManager::Run(const mu_size maxProcessing)
{
	auto iter = ResourcesQueue.begin(), end = ResourcesQueue.begin() + glm::min(ResourcesQueue.size(), maxProcessing);
	for (; iter != end; ++iter)
	{
		auto &baseResource = *iter;
		switch (baseResource->Type)
		{
		case NResourceType::Program:
			{
				auto *resource = static_cast<NProgramResourceRequest*>(baseResource.get());
				if (LoadProgram(resource->ID, resource->Vertex, resource->Fragment, resource->ResourceID, resource->Macros) == false)
				{
					return false;
				}
			}
			break;

		case NResourceType::Texture:
			{
				auto *resource = static_cast<NTextureResourceRequest*>(baseResource.get());
				auto texture = MUTextures::Load(resource->Path, MUTextures::CalculateSamplerFlags(resource->Filter, resource->Wrap));
				if (!texture)
				{
					mu_error("failed to load texture ({})", resource->Path);
					return false;
				}

				TexturesLookup.insert(std::pair(resource->ID, std::move(texture)));
			}
			break;

		case NResourceType::Model:
			{
				auto *resource = static_cast<NModelResourceRequest*>(baseResource.get());
				NModelPtr model(new_nothrow NModel());
				if (model->Load(resource->ID, resource->Path) == false)
				{
					mu_error("failed to load model ({})", resource->Path);
					return false;
				}

				ModelsLookup.insert(std::pair(resource->ID, std::move(model)));
			}
			break;
		}
	}

	end = iter;
	iter = ResourcesQueue.begin();
	if (ResourcesQueue.begin() != end)
	{
		ResourcesQueue.erase(iter, end);
	}

	return true;
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
		[[fallthrough]];
	default: return source;
	}
}

const mu_boolean NResourcesManager::LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, NShaderSettings &settings)
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

	ProgramsLookup.insert(std::pair(id, index));

	return true;
}

const mu_boolean NResourcesManager::LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, const mu_utf8string resourceId, const nlohmann::json &jmacros)
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

void NResourcesManager::LoadPrograms(const mu_utf8string basePath, const nlohmann::json &programs)
{
	const auto dummyArray = nlohmann::json().array();
	ResourcesQueue.reserve(ResourcesQueue.size() + programs.size());
	for (const auto &p : programs)
	{
		const mu_utf8string id = p["id"].get<mu_utf8string>();
		const mu_utf8string vertex = p["vertex"].get<mu_utf8string>();
		const mu_utf8string fragment = p["fragment"].get<mu_utf8string>();
		const mu_utf8string resourceId = p["resource_id"].get<mu_utf8string>();
		const auto &macros = p.contains("macros") ? p["macros"] : dummyArray;

		ResourcesQueue.push_back(
			std::make_unique<NProgramResourceRequest>(id, GameDataPathUTF8 + vertex, GameDataPathUTF8 + fragment, resourceId, macros)
		);
	}
}

void NResourcesManager::LoadTextures(const mu_utf8string basePath, const nlohmann::json &textures)
{
	ResourcesQueue.reserve(ResourcesQueue.size() + textures.size());
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

		ResourcesQueue.push_back(
			std::make_unique<NTextureResourceRequest>(id, GameDataPathUTF8 + path, filter, wrap)
		);
	}
}

void NResourcesManager::LoadModels(const mu_utf8string basePath, const nlohmann::json &models)
{
	ResourcesQueue.reserve(ResourcesQueue.size() + models.size());
	for (const auto &m : models)
	{
		const mu_utf8string id = m["id"];
		const mu_utf8string path = m["path"];

		ResourcesQueue.push_back(
			std::make_unique<NModelResourceRequest>(id, GameDataPathUTF8 + path)
		);
	}
}

const mu_shader NResourcesManager::GetProgram(const mu_utf8string id)
{
	auto iter = ProgramsLookup.find(id);
	if (iter == ProgramsLookup.end()) return NInvalidShader;
	return iter->second;
}

NGraphicsTexture *NResourcesManager::GetTexture(const mu_utf8string id)
{
	auto iter = TexturesLookup.find(id);
	if (iter == TexturesLookup.end()) return nullptr;
	return iter->second.get();
}

NModel *NResourcesManager::GetModel(const mu_utf8string id)
{
	auto iter = ModelsLookup.find(id);
	if (iter == ModelsLookup.end()) return nullptr;
	return iter->second.get();
}

mu_shader NResourcesManager::RegisterShader(NCombinedShader shader)
{
	const mu_shader index = static_cast<mu_shader>(Shaders.size());
	Shaders.push_back(shader);
	return index;
}

NCombinedShader *NResourcesManager::GetShader(const mu_shader shader)
{
	return &Shaders[shader];
}

NTextureAttachmentType NResourcesManager::CreateAttachmentTypeByCRC32(const mu_uint32 key)
{
	auto iter = AttachmentsMap.find(key);
	if (iter != AttachmentsMap.end()) return NInvalidAttachment;
	const auto type = static_cast<NTextureAttachmentType>(AttachmentsMap.size());
	AttachmentsMap.insert(std::make_pair(key, type));
	return type;
}

const mu_size NResourcesManager::GetAttachmentsCount()
{
	return AttachmentsMap.size();
}

const NTextureAttachmentType NResourcesManager::GetAttachmentTypeFromCRC32(const mu_uint32 key)
{
	auto iter = AttachmentsMap.find(key);
	if (iter == AttachmentsMap.end()) return NInvalidAttachment;
	return iter->second;
}

namespace MUResourcesManager
{
	NResourcesManager *CurrentManager = nullptr;

	void SetResourcesManager(NResourcesManager *manager)
	{
		CurrentManager = manager;
	}

	NResourcesManager *GetResourcesManager()
	{
		return CurrentManager;
	}
};

const mu_uint32 GetAttachmentTypeFromId(const mu_char *id)
{
	constexpr mu_size RequiredLength = 32u;
	const mu_size length = glm::min(std::string::traits_type::length(id), RequiredLength);
	return scalopus::crcdetail::compute_padding(id, length, RequiredLength - length);
}