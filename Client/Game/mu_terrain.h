#ifndef __MU_TERRAIN_H__
#define __MU_TERRAIN_H__

#pragma once

constexpr mu_float TerrainScale = 100.0f;
constexpr mu_float TerrainScaleInv = 1.0f / TerrainScale;
constexpr mu_uint32 TerrainSize = 256u; // MU has a fixed terrain size of 256x256
constexpr mu_uint32 TerrainMask = TerrainSize - 1;
constexpr mu_float TerrainSizeInv = 1.0f / static_cast<mu_float>(TerrainSize);

NEXTMU_INLINE mu_uint32 GetTerrainIndex(const mu_uint32 x, const mu_uint32 y)
{
	return y * TerrainSize + x;
}

NEXTMU_INLINE mu_uint32 GetTerrainMaskIndex(const mu_uint32 x, const mu_uint32 y)
{
	return (y & TerrainMask) * TerrainSize + (x & TerrainMask);
}

typedef glm::u8vec4 MappingFormat;

struct TerrainSettings
{
	mu_float WaterMove = 1.0f;
	mu_float WindScale = 1.0f;
	mu_float WindSpeed = 1.0f;
	mu_float Dummy = 0.0f;
};

namespace TerrainAttribute
{
	typedef mu_uint16 Type;
	constexpr auto Format = Diligent::TEX_FORMAT_R16_UINT;
	constexpr auto Stride = TerrainSize * sizeof(mu_uint16);
	enum : Type
	{
		SafeZone = 1 << 0,
		Character = 1 << 1,
		NoMove = 1 << 2,
		NoGround = 1 << 3,
		Water = 1 << 4,
		Action = 1 << 5,
		Height = 1 << 6,
		CameraUp = 1 << 7,
		NoAttackZone = 1 << 8,
	};
};

class NTerrain
{
public:
	~NTerrain();

private:
	void Destroy();

protected:
	friend class NEnvironment;
	const mu_boolean LoadHeightmap(mu_utf8string path);
	const mu_boolean GenerateNormal();
	const mu_boolean LoadLightmap(mu_utf8string path);
	const mu_boolean LoadTextures(
		const mu_utf8string dir,
		const nlohmann::json paths,
		const mu_utf8string filter,
		const mu_utf8string wrap,
		const mu_float uvNormal,
		const mu_float uvScaled,
		std::map<mu_uint32, mu_uint32> &texturesMap
	);
	const mu_boolean LoadGrassTextures(
		const mu_utf8string dir,
		const nlohmann::json paths,
		const mu_utf8string filter,
		const mu_utf8string wrap,
		std::map<mu_uint32, mu_uint32> &texturesMap
	);
	const mu_boolean LoadMappings(
		mu_utf8string path,
		const std::map<mu_uint32, mu_uint32> &texturesMap,
		const std::map<mu_uint32, mu_uint32> &grassTexturesMap
	);
	const mu_boolean LoadAttributes(mu_utf8string path);
	const mu_boolean PrepareSettings(const mu_utf8string path, const nlohmann::json document);
	const mu_boolean GenerateBuffers();
	const mu_boolean PreparePipelines();

public:
	void Reset();
	void ConfigureUniforms();
	void Update();
	void Render();

	Diligent::ITexture *GetLightmapTexture();

	const mu_float GetHeight(const mu_uint32 x, const mu_uint32 y);
	const glm::vec3 GetLight(const mu_uint32 x, const mu_uint32 y);
	const glm::vec3 GetPrimaryLight(const mu_uint32 x, const mu_uint32 y);
	const glm::vec3 GetNormal(const mu_uint32 x, const mu_uint32 y);
	const TerrainAttribute::Type GetAttribute(const mu_uint32 x, const mu_uint32 y);

	const glm::vec3 CalculatePrimaryLight(const mu_float x, const mu_float y);
	const glm::vec3 CalculateBackLight(const mu_float x, const mu_float y);

public:
	const mu_utf8string GetId() const
	{
		return Id;
	}

	const glm::vec4 &GetLightPosition() const
	{
		return LightPosition;
	}

private:
	mu_utf8string Id;

	mu_shader Program = NInvalidShader;
	mu_shader GrassProgram = NInvalidShader;
	NPipelineState *TerrainPipeline = nullptr;
	NPipelineState *GrassPipeline = nullptr;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> TerrainBinding;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> GrassBinding;

	Diligent::RefCntAutoPtr<Diligent::ITexture> HeightmapTexture;
	Diligent::RefCntAutoPtr<Diligent::ITexture> LightmapTexture;
	Diligent::RefCntAutoPtr<Diligent::ITexture> NormalTexture;
	Diligent::RefCntAutoPtr<Diligent::ITexture> MappingTexture;
	Diligent::RefCntAutoPtr<Diligent::ITexture> AttributesTexture;
	Diligent::RefCntAutoPtr<Diligent::ITexture> Textures;
	Diligent::RefCntAutoPtr<Diligent::ITexture> GrassTextures;
	Diligent::RefCntAutoPtr<Diligent::ITexture> UVTexture;
	Diligent::RefCntAutoPtr<Diligent::ITexture> GrassUVTexture;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> SettingsUniform;

	std::unique_ptr<mu_uint8[]> LightmapMemory;
	std::unique_ptr<mu_uint8[]> NormalMemory;

	mu_float HeightMultiplier = 1.0f;
	glm::vec3 Light = glm::vec3();
	glm::vec4 LightPosition = glm::vec4(1.3f, 0.0f, 2.0f, 1.0f);

	mu_float WaterModulus = 1.0f;
	mu_float WaterMultiplier = 1.0f;

	mu_float WindScale = 1.0f;
	mu_float WindModulus = 1.0f;
	mu_float WindMultiplier = 1.0f;

	std::unique_ptr<mu_float[]> TerrainHeight;
	std::unique_ptr<glm::vec3[]> TerrainLight;
	std::unique_ptr<glm::vec3[]> TerrainPrimaryLight;
	std::unique_ptr<glm::vec3[]> TerrainNormal;
	std::unique_ptr<TerrainAttribute::Type[]> TerrainAttributes;

	TerrainSettings Settings;
};

#endif