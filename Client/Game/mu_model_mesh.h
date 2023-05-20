#ifndef __MU_MODEL_MESH_H__
#define __MU_MODEL_MESH_H__

#pragma once

#include <array>
#include <vector>
#include "t_graphics.h"

struct NVertex
{
	mu_int16 Node = NInvalidInt16;
	glm::vec3 Position = glm::vec3();
};

struct NNormal
{
	mu_int16 Node = NInvalidInt16;
	glm::vec3 Normal = glm::vec3();
};

struct NTexCoord
{
	glm::vec2 UV = glm::vec2();
};

struct NTriangle
{
	std::array<mu_int16, 3> Vertices;
	std::array<mu_int16, 3> Normals;
	std::array<mu_int16, 3> TexCoords;
};

struct NRenderInfo
{
	mu_uint32 Offset = 0;
	mu_uint32 Count = 0;
};

struct NTextureInfo
{
	mu_boolean ForceFilter = false;
	mu_boolean ForceWrap = false;
	mu_utf8string Filename;
	mu_utf8string Filter = "nearest";
	mu_utf8string Wrap = "repeat";
};

namespace ModelRenderMode
{
	enum : mu_uint32
	{
		Normal,
		Alpha,
		Count,
	};
};

struct NMeshRenderSettings
{
	mu_shader Program = NInvalidShader;
	mu_shader ShadowProgram = NInvalidShader;
	NGraphicsTexture *Texture = nullptr;
	NGraphicsTexture *VertexTexture = nullptr;
	NDynamicPipelineState RenderState[ModelRenderMode::Count] = { DefaultDynamicPipelineState, DefaultAlphaDynamicPipelineState };
	NDynamicPipelineState ShadowRenderState[ModelRenderMode::Count] = { DefaultShadowDynamicPipelineState, DefaultShadowDynamicPipelineState };
	NRenderClassify ClassifyMode = NRenderClassify::None;
	mu_uint32 ClassifyIndex = 0;
	mu_float Light = 1.0f;
	mu_float AlphaTest = 0.25f;
	mu_boolean PremultiplyLight = false;
	mu_boolean PremultiplyAlpha = false;
};

class NMesh
{
public:
	NMeshRenderSettings Settings;
	NRenderInfo VertexBuffer;
	//NRenderInfo IndexBuffer; // Not required for now, since we will use vertices only for the rendering
	NTextureInfo Texture;
	std::vector<NVertex> Vertices;
	std::vector<NNormal> Normals;
	std::vector<NTexCoord> TexCoords;
	std::vector<NTriangle> Triangles;
};

class NVirtualMesh
{
public:
	mu_uint32 Mesh;
	NMeshRenderSettings Settings;
};

#endif