#ifndef __MU_RESOURCESMANAGER_H__
#define __MU_RESOURCESMANAGER_H__

#pragma once

#include "mu_resources.h"
#include <ShaderMacroHelper.hpp>

class NModel;

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

namespace MUResourcesManager
{
	const mu_boolean Load(const mu_utf8string jsonFilename = "resources.json");
	void Destroy();

	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, NShaderSettings &settings);
	const mu_boolean LoadProgram(const mu_utf8string id, const mu_utf8string vertex, const mu_utf8string fragment, const mu_utf8string resourceId, const nlohmann::json &jmacros);

	const mu_shader GetProgram(const mu_utf8string id);
	NGraphicsTexture *GetTexture(const mu_utf8string id);
	NModel *GetModel(const mu_utf8string id);
};

#endif