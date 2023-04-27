#include "stdafx.h"
#include "t_graphics_pipelineresources.h"
#include "t_graphics_immutables.h"

std::map<mu_utf8string, NPipelineResource> Resources;

void CreatePipelineResources()
{
	// Mesh
	{
		NPipelineResource resource;

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"cbCameraAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL,
				"cbLightAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"ModelViewProj",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_SkeletonTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		/*resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_LightTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);*/

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL,
				"ModelSettings",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_Texture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_tex2DShadowMap",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_tex2DFilterableShadowMap",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		Resources.insert(std::make_pair("mesh", resource));
	}

	// Terrain
	{
		NPipelineResource resource;

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"cbCameraAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL,
				"cbLightAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_HeightTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_LightTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_NormalTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_MappingTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_UVTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_AttributesTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"TerrainSettings",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_Textures",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_tex2DShadowMap",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_tex2DFilterableShadowMap",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		Resources.insert(std::make_pair("terrain", resource));
	}

	// Grass
	{
		NPipelineResource resource;

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"cbCameraAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL,
				"cbLightAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_HeightTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_LightTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_NormalTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_MappingTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_UVTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"g_AttributesTexture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"TerrainSettings",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_Textures",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_tex2DShadowMap",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_tex2DFilterableShadowMap",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		Resources.insert(std::make_pair("grass", resource));
	}

	// Joints
	{
		NPipelineResource resource;

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"cbCameraAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"JointSettings",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_Texture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		Resources.insert(std::make_pair("joint", resource));
	}

	// Particles
	{
		NPipelineResource resource;

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"cbCameraAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"ParticleSettings",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_PIXEL,
				"g_Texture",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
			)
		);

		Resources.insert(std::make_pair("particle", resource));
	}

	// Bounding Box
	{
		NPipelineResource resource;

		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"cbCameraAttribs",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);
		resource.Variables.push_back(
			Diligent::ShaderResourceVariableDesc(
				Diligent::SHADER_TYPE_VERTEX,
				"BBoxDimensions",
				Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC
			)
		);

		Resources.insert(std::make_pair("bbox", resource));
	}
}

const NPipelineResource *GetPipelineResource(const mu_utf8string resourceId)
{
	auto iter = Resources.find(resourceId);
	if (iter == Resources.end()) return nullptr;
	return &iter->second;
}