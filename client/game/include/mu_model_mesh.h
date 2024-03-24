#ifndef __MU_MODEL_MESH_H__
#define __MU_MODEL_MESH_H__

#pragma once

#include <array>
#include <vector>
#include "t_graphics.h"
#include "res_item.h"

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

enum class EMeshRenderConditionType : mu_uint8
{
	ItemLevel, // Direct Item Level
	ItemLevelByFormula, // Item Level calculated by formula (the formula takes the item level and converts it into another range like 0~15 which is the original level system)
	ItemRank, // Item level calculated rank by configured formula (>=0)
	ItemOptionsCount, // Item options count
	ItemOption, // Check if specific option exists
	ItemOptionsMinRank, // Item option calculated minimum rank by configured formula (>=0)
	ItemOptionsMaxRank, // Item option calculated maximum rank by configured formula (>=0)
	ItemOptionsAvgRank, // Item option calculated average rank by configured formula (>=0)
};

enum class EMeshRenderConditionOperator : mu_uint8
{
	Equal,
	NotEqual,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
};

class NMeshRenderConditionInput
{
public:
	mu_uint8 Level;
	mu_uint8 LevelByFormula;
	mu_uint8 OptionsCount;

	EItemRank Rank;
	EItemRank OptionsMinRank;
	EItemRank OptionsMaxRank;
	EItemRank OptionsAvgRank;

	std::vector<EItemOption> OptionsType;
};

class NMeshRenderCondition
{
public:
	EMeshRenderConditionType Type;
	EMeshRenderConditionOperator Operator;
	union {
		mu_float FValue;
		mu_int32 SValue;
		mu_uint32 UValue;
	};
};

typedef std::vector<NMeshRenderCondition> NMeshRenderConditions;

enum class EMeshRenderLightType : mu_uint8
{
	BlendAdd,
	BlendSubtract,
	BlendMultiply,
	BlendDivide, // Source / Value
	BlendInverseDivide, // Value / Source
	SourceSet,
	TargetSet,
};

enum class EMeshRenderLightSource : mu_uint8
{
	None, // 1, 1, 1
	Light, // Input Light
	Luminosity,
};

class NRenderVirtualMeshLight
{
public:
	mu_boolean ShouldClamp; // If enabled it will clamp every value to [0,1]

	EMeshRenderLightType PreType;
	EMeshRenderLightSource PreSource;
	glm::vec3 PreValue;

	EMeshRenderLightType PostType;
	EMeshRenderLightSource PostSource;
	glm::vec3 PostValue;

	std::vector<NMeshRenderConditions> Conditions;
};

struct NMeshRenderInheritConfig
{
	mu_boolean BlendMeshLight = false;
};

struct NMeshRenderSettings
{
	mu_shader Program = NInvalidShader;
	mu_shader ShadowProgram = NInvalidShader;
	ASModuleScript Script;
	AngelScript::asIScriptFunction *ScriptFunction = nullptr;
	NGraphicsTexture *Texture = nullptr;
	NGraphicsTexture *VertexTexture = nullptr;
	NDynamicPipelineState RenderState[ModelRenderMode::Count] = { DefaultDynamicPipelineState, DefaultAlphaDynamicPipelineState };
	NDynamicPipelineState ShadowRenderState[ModelRenderMode::Count] = { DefaultShadowDynamicPipelineState, DefaultShadowDynamicPipelineState };
	NRenderClassify ClassifyMode = NRenderClassify::None;
	mu_uint32 ClassifyIndex = 0;
	mu_float AlphaTest = 0.25f;
	mu_boolean PremultiplyLight = false;
	mu_boolean PremultiplyAlpha = false;
	std::vector<NRenderVirtualMeshLight> Lights;
	NMeshRenderInheritConfig Inherit;
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
	std::vector<NMeshRenderConditions> Conditions;
};

#endif