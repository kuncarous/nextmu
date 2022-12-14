#ifndef __MU_TERRAIN_H__
#define __MU_TERRAIN_H__

#pragma once

constexpr mu_float TerrainScale = 100.0f;
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
	constexpr auto Format = bgfx::TextureFormat::R16U;
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

	const mu_boolean Initialize(mu_utf8string path);
	void Destroy();

private:
	const mu_boolean Load(const mu_utf8string path);
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
	const mu_boolean LoadMappings(mu_utf8string path, const std::map<mu_uint32, mu_uint32> &texturesMap);
	const mu_boolean LoadAttributes(mu_utf8string path);
	const mu_boolean PrepareSettings(const mu_utf8string path, const nlohmann::json document);

public:
	void Reset();
	void ConfigureUniforms();
	void Update();
	void Render();

	const bgfx::TextureHandle GetLightmapTexture() const;
	const bgfx::UniformHandle GetLightmapSampler() const;

	const mu_float GetHeight(const mu_uint32 x, const mu_uint32 y);
	const glm::vec3 GetLight(const mu_uint32 x, const mu_uint32 y);
	const glm::vec3 GetPrimaryLight(const mu_uint32 x, const mu_uint32 y);
	const glm::vec3 GetNormal(const mu_uint32 x, const mu_uint32 y);
	const TerrainAttribute::Type GetAttribute(const mu_uint32 x, const mu_uint32 y);

private:
	bgfx::ProgramHandle Program = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle HeightmapSampler = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle HeightmapTexture = BGFX_INVALID_HANDLE;
	std::unique_ptr<mu_uint8[]> LightmapMemory;
	bgfx::UniformHandle LightmapSampler = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle LightmapTexture = BGFX_INVALID_HANDLE;
	std::unique_ptr<mu_uint8[]> NormalMemory;
	bgfx::UniformHandle NormalSampler = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle NormalTexture = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle MappingSampler = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle MappingTexture = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle AttributesSampler = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle AttributesTexture = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle TexturesSampler = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle Textures = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle UVSampler = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle UVTexture = BGFX_INVALID_HANDLE;
	bgfx::VertexBufferHandle VertexBuffer = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle IndexBuffer = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle SettingsUniform = BGFX_INVALID_HANDLE;

	mu_float HeightMultiplier = 1.0f;
	glm::vec3 Light = glm::vec3();

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