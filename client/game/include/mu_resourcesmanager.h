#ifndef __MU_RESOURCESMANAGER_H__
#define __MU_RESOURCESMANAGER_H__

#pragma once

#include "mu_resources.h"
#include "t_textureattachments.h"
#include <ShaderMacroHelper.hpp>

class NModel;
class NGraphicsTexture;

typedef std::unique_ptr<NModel> NModelPtr;
typedef std::unique_ptr<NGraphicsTexture> NGraphicsTexturePtr;

struct NShaderSettings
{
	Diligent::ShaderMacroHelper VertexMacros;
	Diligent::ShaderMacroHelper PixelMacros;
	Diligent::InputLayoutDesc InputLayout;
	mu_utf8string AppendVertex;
	mu_utf8string AppendPixel;
	const NPipelineResource *Resource = nullptr;
	std::vector<Diligent::IPipelineResourceSignature *> ResourceSignatures;
};

namespace UINoesis
{
	class DERenderDevice;
};

class NResourcesManager
{
public:
	static const mu_size MaxResourcesProcessingPerFrame = 10;
	static const mu_size BlockingResourcesProcessingPerFrame = std::numeric_limits<mu_size>::max();

public:
	~NResourcesManager() { Destroy(); }

public:
	const mu_boolean Load(const mu_utf8string filename);
	void Destroy();
	const mu_boolean Run(const mu_size maxProcessing = MaxResourcesProcessingPerFrame);
	const mu_boolean RunAndWait()
	{
		return Run(BlockingResourcesProcessingPerFrame);
	}

private:
	friend class UINoesis::DERenderDevice;
	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, NShaderSettings &settings);
	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, const mu_utf8string resourceId, const nlohmann::json &jmacros);

private:
	void LoadPrograms(const mu_utf8string basePath, const nlohmann::json &programs);
	void LoadTextures(const mu_utf8string basePath, const nlohmann::json &textures);
	void LoadModels(const mu_utf8string basePath, const nlohmann::json &models);

public:
	const mu_shader GetProgram(const mu_utf8string id);
	NGraphicsTexture *GetTexture(const mu_utf8string id);
	NModel *GetModel(const mu_utf8string id);

private:
	mu_shader RegisterShader(NCombinedShader shader);

public:
	NCombinedShader *GetShader(const mu_shader shader);

private:
	NTextureAttachmentType CreateAttachmentTypeByCRC32(const mu_uint32 key);

public:
	const mu_size GetAttachmentsCount();
	const NTextureAttachmentType GetAttachmentTypeFromCRC32(const mu_uint32 key);

private:
	std::map<mu_utf8string, mu_shader> ProgramsLookup;
	std::map<mu_utf8string, NGraphicsTexturePtr> TexturesLookup;
	std::map<mu_utf8string, NModelPtr> ModelsLookup;
	std::vector<NCombinedShader> Shaders;
	std::map<mu_uint32, NTextureAttachmentType> AttachmentsMap;

public:
	const mu_boolean IsResourcesQueueEmpty() const
	{
		return ResourcesQueue.empty();
	}

	const mu_size GetResourcesToLoad() const
	{
		return ResourcesToLoad;
	}

	const mu_size GetResourcesLoaded() const
	{
		return ResourcesToLoad - ResourcesQueue.size();
	}

private:
	std::vector<NBaseResourceRequestPtr> ResourcesQueue;
	mu_size ResourcesToLoad = 0;
};

namespace MUResourcesManager
{
	void SetResourcesManager(NResourcesManager *manager);
	NResourcesManager *GetResourcesManager();
};

const mu_uint32 GetAttachmentTypeFromId(const mu_char* id);

#define GenerateAttachmentTypeAtCompileTime_L(id, length) CRC32_DATA_PADDING(id, length, 32u - length)
#define GenerateAttachmentTypeAtCompileTime(id) GenerateAttachmentTypeAtCompileTime_L(id, GetStringLengthAtCompileTime(id))

#endif