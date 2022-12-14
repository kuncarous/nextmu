#include "stdafx.h"
#include "mu_terrain.h"
#include "mu_graphics.h"
#include "mu_textures.h"
#include "mu_resourcesmanager.h"
#include "mu_state.h"
#include "mu_crypt.h"
#include "mu_binaryreader.h"
#include <glm/gtx/normal.hpp>
#include <glm/gtc/packing.hpp>

static bgfx::VertexLayout VertexLayout;

static void InitializeVertexLayout()
{
	static mu_boolean initialized = false;
	if (initialized) return;
	initialized = true;
	VertexLayout
		.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Uint8)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Uint8)
		.end();
}

struct TerrainVertex
{
	mu_uint8 x, y;
	mu_uint8 rx, ry;
};

TerrainVertex TerrainVertices[4 * TerrainSize * TerrainSize] = {};

void InitializeTerrainVertices()
{
	static mu_boolean initialized = false;
	if (initialized) return;
	initialized = true;

	mu_uint32 vertex = 0;
	for (mu_uint32 y = 0; y < TerrainSize; ++y)
	{
		for (mu_uint32 x = 0; x < TerrainSize; ++x)
		{
			TerrainVertex *vert = &TerrainVertices[vertex++];
			vert->x = x;
			vert->y = y;
			vert->rx = x;
			vert->ry = y;

			vert = &TerrainVertices[vertex++];
			vert->x = x + 1;
			vert->y = y;
			vert->rx = x;
			vert->ry = y;

			vert = &TerrainVertices[vertex++];
			vert->x = x + 1;
			vert->y = y + 1;
			vert->rx = x;
			vert->ry = y;

			vert = &TerrainVertices[vertex++];
			vert->x = x;
			vert->y = y + 1;
			vert->rx = x;
			vert->ry = y;
		}
	}
}

mu_uint32 TerrainIndexes[(TerrainSize - 1) * (TerrainSize - 1) * 6] = {};

void InitializeTerrainIndexes()
{
	static mu_boolean initialized = false;
	if (initialized) return;
	initialized = true;

	mu_uint32 index = 0, vertex = 0;
	for (mu_uint32 y = 0; y < (TerrainSize - 1); ++y)
	{
		for (mu_uint32 x = 0; x < (TerrainSize - 1); ++x)
		{
			TerrainIndexes[index + 0] = vertex + 0;
			TerrainIndexes[index + 1] = vertex + 1;
			TerrainIndexes[index + 2] = vertex + 2;
			TerrainIndexes[index + 3] = vertex + 0;
			TerrainIndexes[index + 4] = vertex + 2;
			TerrainIndexes[index + 5] = vertex + 3;

			index += 6;
			vertex += 4;
		}

		vertex += 4; // Since we are only calculating one less
	}

	index = 0;
}

NTerrain::~NTerrain()
{
	Destroy();
}

const mu_boolean NTerrain::Initialize(mu_utf8string path)
{
	NormalizePath<true>(path);

	if (Load(path) == false)
	{
		Destroy();
		return false;
	}

	return true;
}

void NTerrain::Destroy()
{
#define RELEASE_HANDLER(handler) \
	if (bgfx::isValid(handler)) \
	{ \
		bgfx::destroy(handler); \
		handler = BGFX_INVALID_HANDLE; \
	}

	//RELEASE_HANDLER(Program); // Now the program is being handled by the resources manager
	RELEASE_HANDLER(HeightmapSampler);
	RELEASE_HANDLER(HeightmapTexture);
	RELEASE_HANDLER(LightmapSampler);
	RELEASE_HANDLER(LightmapTexture);
	RELEASE_HANDLER(NormalSampler);
	RELEASE_HANDLER(NormalTexture);
	RELEASE_HANDLER(MappingSampler);
	RELEASE_HANDLER(MappingTexture);
	RELEASE_HANDLER(AttributesSampler);
	RELEASE_HANDLER(AttributesTexture);
	RELEASE_HANDLER(TexturesSampler);
	RELEASE_HANDLER(Textures);
	RELEASE_HANDLER(UVSampler);
	RELEASE_HANDLER(UVTexture);
	RELEASE_HANDLER(VertexBuffer);
	RELEASE_HANDLER(IndexBuffer);
	RELEASE_HANDLER(SettingsUniform);
}

const mu_boolean NTerrain::Load(const mu_utf8string path)
{
	const mu_utf8string filename = path + "terrain.json";
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
		mu_error("terrain.json malformed ({})", filename);
		return false;
	}

	const auto programId = document["program"].get<mu_utf8string>();
	Program = MUResourcesManager::GetProgram(programId);
	if (bgfx::isValid(Program) == false)
	{
		mu_error("terrain program not found ({}, {})", filename, programId);
		return false;
	}

	HeightMultiplier = document["height_multiplier"].get<mu_float>();

	const auto heightmap = document["heightmap"].get<mu_utf8string>();
	if (LoadHeightmap(path + heightmap) == false)
	{
		mu_error("failed to load heightmap ({})", path + heightmap);
		return false;
	}

	if (GenerateNormal() == false)
	{
		mu_error("failed to generate normal ({})", filename);
		return false;
	}

	const auto lightmap = document["lightmap"].get<mu_utf8string>();
	if (LoadLightmap(path + lightmap) == false)
	{
		mu_error("failed to load lightmap ({})", path + lightmap);
		return false;
	}

	const auto light = document["light"];
	if (light.is_array() == false || light.size() != 3)
	{
		mu_error("invalid terrain light ({})", path + lightmap);
		return false;
	}

	for (mu_uint32 n = 0; n < 3; ++n)
	{
		Light[n] = light[n].get<mu_float>();
	}

	InitializeVertexLayout();
	InitializeTerrainVertices();
	InitializeTerrainIndexes();

	const bgfx::Memory *mem = bgfx::makeRef(TerrainVertices, sizeof(TerrainVertices));
	VertexBuffer = bgfx::createVertexBuffer(mem, VertexLayout);
	if (bgfx::isValid(VertexBuffer) == false)
	{
		mu_error("failed to create vertex buffer ({})", filename);
		return false;
	}

	mem = bgfx::makeRef(TerrainIndexes, sizeof(TerrainIndexes));
	IndexBuffer = bgfx::createIndexBuffer(mem, BGFX_BUFFER_INDEX32);
	if (bgfx::isValid(IndexBuffer) == false)
	{
		mu_error("failed to create index buffer ({})", filename);
		return false;
	}

	const auto uv = document["uv"];
	if (uv.is_object() == false)
	{
		mu_error("terrain.json malformed ({})", filename);
		return false;
	}

	const auto uvNormal = uv["normal"].get<mu_float>();
	const auto uvScaled = uv["scaled"].get<mu_float>();

	const auto filter = document["filter"].get<mu_utf8string>();
	const auto wrap = document["wrap"].get<mu_utf8string>();

	const auto textures = document["textures"];
	if (textures.is_array() == false)
	{
		mu_error("terrain.json textures missing ({})", filename);
		return false;
	}

	std::map<mu_uint32, mu_uint32> texturesMap;
	if (LoadTextures(path, textures, filter, wrap, uvNormal, uvScaled, texturesMap) == false)
	{
		mu_error("failed to load textures ({})", filename);
		return false;
	}

	const auto mappings = document["mappings"].get<mu_utf8string>();
	if (LoadMappings(path + mappings, texturesMap) == false)
	{
		mu_error("failed to load mappings ({})", path + mappings);
		return false;
	}

	const auto attributes = document["attributes"].get<mu_utf8string>();
	if (LoadAttributes(path + attributes) == false)
	{
		mu_error("failed to load attributes ({})", path + attributes);
		return false;
	}

	if (PrepareSettings(path, document) == false)
	{
		return false;
	}

	return true;
}

const mu_boolean NTerrain::LoadHeightmap(mu_utf8string path)
{
	NormalizePath(path);

	auto ext = path.substr(path.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
	{
		mu_error("heightmap not found ({})", path);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));

	if (ext == "ozb")
	{
		fileLength -= 4 + 1080;
		SDL_RWseek(fp, 4 + 1080, RW_SEEK_CUR);
	}

	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	/*
		OZB file won't be loaded by free image, the image result will be moved by a few pixels,
		instead we load it directly form the buffer.
	*/
	if (ext != "ozb")
	{
		FIMEMORY *memory = FreeImage_OpenMemory(buffer.get(), static_cast<DWORD>(fileLength));
		if (memory == nullptr)
		{
			return false;
		}

		FIBITMAP *bitmap = FreeImage_LoadFromMemory(FREE_IMAGE_FORMAT::FIF_BMP, memory, PNG_IGNOREGAMMA);
		buffer.reset();
		FreeImage_CloseMemory(memory);

		if (bitmap == nullptr)
		{
			return false;
		}

		const FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(bitmap);

		// Only 8bits textures are supported
		if (imageType != FIT_BITMAP)
		{
			FreeImage_Unload(bitmap);
			return false;
		}

		const mu_uint32 width = FreeImage_GetWidth(bitmap);
		const mu_uint32 height = FreeImage_GetHeight(bitmap);
		if (width != TerrainSize || height != TerrainSize)
		{
			FreeImage_Unload(bitmap);
			return false;
		}

		const mu_uint32 pitch = FreeImage_GetPitch(bitmap);
		if (pitch != TerrainSize) // Pitch should be same as TerrainSize since it is a grayscale texture
		{
			FreeImage_Unload(bitmap);
			return false;
		}

		const mu_uint32 bpp = FreeImage_GetBPP(bitmap);
		if (bpp != 8) // Only 8 bits per pixel is supported
		{
			FreeImage_Unload(bitmap);
			return false;
		}

		const mu_uint8 *bitmapBuffer = FreeImage_GetBits(bitmap);
		buffer.reset(new_nothrow mu_uint8[TerrainSize * TerrainSize]);
		mu_memcpy(buffer.get(), bitmapBuffer, TerrainSize * TerrainSize);

		FreeImage_Unload(bitmap);
	}

	if (!TerrainHeight) TerrainHeight.reset(new_nothrow mu_float[TerrainSize * TerrainSize]);
	for (mu_uint32 index = 0; index < TerrainSize * TerrainSize; ++index)
	{
		TerrainHeight[index] = static_cast<mu_float>(buffer[index]) * HeightMultiplier;
	}

	const bgfx::Memory *mem = bgfx::copy(buffer.get(), TerrainSize * TerrainSize);
	HeightmapTexture = bgfx::createTexture2D(TerrainSize, TerrainSize, false, 1, bgfx::TextureFormat::R8U, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT, mem);
	HeightmapSampler = bgfx::createUniform("s_heightTexture", bgfx::UniformType::Sampler);

	return true;
}

template<typename T, glm::qualifier Q>
GLM_FUNC_QUALIFIER glm::vec<3, T, Q> triangleNormalMU
(
	glm::vec<3, T, Q> const &p1,
	glm::vec<3, T, Q> const &p2,
	glm::vec<3, T, Q> const &p3
)
{
	return normalize(cross(p2 - p1, p3 - p1));
}

const mu_boolean NTerrain::GenerateNormal()
{
	if (!TerrainNormal) TerrainNormal.reset(new_nothrow glm::vec3[TerrainSize * TerrainSize]);
	glm::vec3 *dest = TerrainNormal.get();
	for (mu_uint32 y = 0; y < TerrainSize; ++y)
	{
		for (mu_uint32 x = 0; x < TerrainSize; ++x, ++dest)
		{
			*dest = triangleNormalMU(
				glm::vec3((x + 1) * TerrainScale, y * TerrainScale, GetHeight(x + 1, y)),
				glm::vec3((x + 1) * TerrainScale, (y + 1) * TerrainScale, GetHeight(x + 1, y + 1)),
				glm::vec3(x * TerrainScale, (y + 1) * TerrainScale, GetHeight(x, y + 1))
			);
		}
	}

	if (!NormalMemory) NormalMemory.reset(new_nothrow mu_uint8[sizeof(mu_uint16) * 4 * TerrainSize * TerrainSize]);
	mu_uint64 *data = reinterpret_cast<mu_uint64 *>(NormalMemory.get());
	glm::vec3 *src = TerrainNormal.get();
	for (mu_uint32 y = 0; y < TerrainSize; ++y)
	{
		for (mu_uint32 x = 0; x < TerrainSize; ++x, ++src, ++data)
		{
			*data = glm::packUnorm4x16(glm::vec4(*src, 0.0f));
		}
	}

	NormalTexture = bgfx::createTexture2D(TerrainSize, TerrainSize, false, 1, bgfx::TextureFormat::RGBA16, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT);
	NormalSampler = bgfx::createUniform("s_normalTexture", bgfx::UniformType::Sampler);

	const bgfx::Memory *mem = bgfx::makeRef(NormalMemory.get(), sizeof(mu_uint16) * 4 * TerrainSize * TerrainSize);
	bgfx::updateTexture2D(NormalTexture, 0, 0, 0, 0, TerrainSize, TerrainSize, mem);

	return true;
}

const mu_boolean NTerrain::LoadLightmap(mu_utf8string path)
{
	NormalizePath(path);

	auto ext = path.substr(path.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
	{
		mu_error("heightmap not found ({})", path);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));

	if (ext == "ozj")
	{
		fileLength -= 24;
		SDL_RWseek(fp, 24, RW_SEEK_CUR);
	}

	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	FIMEMORY *memory = FreeImage_OpenMemory(buffer.get(), static_cast<DWORD>(fileLength));
	if (memory == nullptr)
	{
		return false;
	}

	FIBITMAP *bitmap = FreeImage_LoadFromMemory(FREE_IMAGE_FORMAT::FIF_JPEG, memory, PNG_IGNOREGAMMA);
	buffer.reset();
	FreeImage_CloseMemory(memory);

	if (bitmap == nullptr)
	{
		return false;
	}

	const FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(bitmap);
	if (imageType != FIT_BITMAP)
	{
		FreeImage_Unload(bitmap);
		return false;
	}

	const mu_uint32 width = FreeImage_GetWidth(bitmap);
	const mu_uint32 height = FreeImage_GetHeight(bitmap);
	if (width != TerrainSize || height != TerrainSize)
	{
		FreeImage_Unload(bitmap);
		return false;
	}

	const mu_uint32 pitch = FreeImage_GetPitch(bitmap);
	const mu_uint32 bpp = FreeImage_GetBPP(bitmap);
	if (pitch != TerrainSize * 4 || bpp != 32)
	{
		FIBITMAP *newBitmap = FreeImage_ConvertTo32Bits(bitmap);
		FreeImage_Unload(bitmap);

		if (newBitmap == nullptr)
		{
			return false;
		}

		bitmap = newBitmap;
	}

	const mu_uint8 *bitmapBuffer = FreeImage_GetBits(bitmap);
	if (!LightmapMemory) LightmapMemory.reset(new_nothrow mu_uint8[sizeof(mu_uint8) * 4 * TerrainSize * TerrainSize]);
	mu_memcpy(LightmapMemory.get(), bitmapBuffer, sizeof(mu_uint8) * 4 * TerrainSize * TerrainSize);
	FreeImage_Unload(bitmap);

	if (!TerrainLight) TerrainLight.reset(new_nothrow glm::vec3[TerrainSize * TerrainSize]);
	if (!TerrainPrimaryLight) TerrainPrimaryLight.reset(new_nothrow glm::vec3[TerrainSize * TerrainSize]);
	const mu_uint32 *lightBuffer = reinterpret_cast<const mu_uint32 *>(LightmapMemory.get());
	const glm::vec3 *normalBuffer = TerrainNormal.get();
	for (mu_uint32 index = 0; index < TerrainSize * TerrainSize; ++index, ++lightBuffer, ++normalBuffer)
	{
		glm::vec3 light(glm::unpackUnorm4x8(*lightBuffer));
		TerrainLight[index] = light * glm::clamp(glm::dot(*normalBuffer, Light) + 0.5f, 0.0f, 1.0f);
	}

	LightmapTexture = bgfx::createTexture2D(TerrainSize, TerrainSize, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT);
	LightmapSampler = bgfx::createUniform("s_lightTexture", bgfx::UniformType::Sampler);

	const bgfx::Memory *mem = bgfx::makeRef(LightmapMemory.get(), sizeof(uint8_t) * 4 * TerrainSize * TerrainSize);
	bgfx::updateTexture2D(LightmapTexture, 0, 0, 0, 0, TerrainSize, TerrainSize, mem);

	return true;
}

NEXTMU_INLINE mu_uint32 GetPowerOfTwoSize(const mu_uint32 size)
{
	mu_uint32 s = 1;
	while (s < size) s <<= 1;
	return s;
}

struct bitmap_delete { // default deleter for unique_ptr
	constexpr bitmap_delete() noexcept = default;

	constexpr void operator()(FIBITMAP *_Ptr) const noexcept /* strengthened */ { // delete a pointer
		if (_Ptr == nullptr) return;
		FreeImage_Unload(_Ptr);
	}
};

typedef std::unique_ptr<FIBITMAP, bitmap_delete> UniqueBitmap;

const mu_boolean NTerrain::LoadTextures(
	const mu_utf8string dir,
	const nlohmann::json textures,
	const mu_utf8string filter,
	const mu_utf8string wrap,
	const mu_float uvNormal,
	const mu_float uvScaled,
	std::map<mu_uint32, mu_uint32> &texturesMap
)
{
	typedef glm::vec4 SettingFormat;
	mu_uint32 width = 0, height = 0;
	std::vector<SettingFormat> settings;
	std::vector<UniqueBitmap> bitmaps;

	for (auto iter = textures.begin(); iter != textures.end(); ++iter)
	{
		const auto &texture = *iter;
		if (texture.is_object() == false)
		{
			return false;
		}

		const auto id = texture["id"].get<mu_uint32>();
		const auto path = texture["path"].get<mu_utf8string>();

		FIBITMAP *bitmap = nullptr;
		if (MUTextures::LoadRaw(dir + path, &bitmap) == false)
		{
			return false;
		}

		mu_uint32 w = FreeImage_GetWidth(bitmap), h = FreeImage_GetHeight(bitmap);
		if (w > width) width = w;
		if (h > height) height = h;

		const auto scaled = texture["scaled"].get<mu_boolean>();
		const auto water = texture["water"].get<mu_boolean>();

		if (!scaled)
		{
			settings.push_back(SettingFormat(uvNormal / static_cast<mu_float>(w), uvNormal / static_cast<mu_float>(h), static_cast<mu_float>(water), 0.0f));
		}
		else
		{
			settings.push_back(SettingFormat(uvScaled / static_cast<mu_float>(w), uvScaled / static_cast<mu_float>(h), static_cast<mu_float>(water), 0.0f));
		}

		texturesMap.insert(std::pair(id, static_cast<mu_uint32>(bitmaps.size())));
		bitmaps.push_back(UniqueBitmap(bitmap));
	}

	width = GetPowerOfTwoSize(width);
	height = GetPowerOfTwoSize(height);

	for (auto iter = bitmaps.begin(); iter != bitmaps.end(); ++iter)
	{
		auto &ubitmap = *iter;
		FIBITMAP *bitmap = ubitmap.get();

		mu_uint32 w = FreeImage_GetWidth(bitmap), h = FreeImage_GetHeight(bitmap);
		if (w < width || h < height)
		{
			FIBITMAP *newBitmap = FreeImage_Rescale(bitmap, width, height, FREE_IMAGE_FILTER::FILTER_BICUBIC);
			if (newBitmap == nullptr)
			{
				return false;
			}

			ubitmap.reset(newBitmap);
			bitmap = newBitmap;
		}
	}

	const mu_uint16 numLayers = static_cast<mu_uint16>(bitmaps.size());
	const mu_uint32 textureSize = width * height * 4;
	const mu_uint32 memorySize = textureSize * numLayers;
	const bgfx::Memory *mem = bgfx::alloc(memorySize);

	mu_uint8 *dest = mem->data;
	for (auto iter = bitmaps.begin(); iter != bitmaps.end(); ++iter)
	{
		auto &bitmap = *iter;
		mu_memcpy(dest, FreeImage_GetBits(bitmap.get()), textureSize);
		dest += textureSize;
	}

	Textures = bgfx::createTexture2D(width, height, false, numLayers, bgfx::TextureFormat::RGBA8, MUTextures::CalculateSamplerFlags(filter, wrap), mem);
	TexturesSampler = bgfx::createUniform("s_textures", bgfx::UniformType::Sampler);

	const mu_uint32 settingsWidth = GetPowerOfTwoSize(static_cast<mu_uint32>(settings.size()));
	mem = bgfx::alloc(settingsWidth * sizeof(SettingFormat));
	mu_zeromem(mem->data, mem->size);
	mu_memcpy(mem->data, settings.data(), settings.size() * sizeof(SettingFormat));
	UVTexture = bgfx::createTexture2D(settingsWidth, 1, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT, mem);
	UVSampler = bgfx::createUniform("s_uvTexture", bgfx::UniformType::Sampler);

	return true;
}

const mu_boolean NTerrain::LoadMappings(mu_utf8string path, const std::map<mu_uint32, mu_uint32> &texturesMap)
{
	NormalizePath(path);

	auto ext = path.substr(path.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
	{
		mu_error("heightmap not found ({})", path);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	XorDecrypt(buffer.get(), buffer.get(), static_cast<mu_uint32>(fileLength));

	NBinaryReader reader(buffer.get(), static_cast<mu_uint32>(fileLength));
	mu_uint8 version = reader.Read<mu_uint8>();
	mu_int32 map = static_cast<mu_int32>(reader.Read<mu_uint8>());
	const mu_uint8 *mapping1 = reader.GetPointer(); reader.Skip(TerrainSize * TerrainSize);
	const mu_uint8 *mapping2 = reader.GetPointer(); reader.Skip(TerrainSize * TerrainSize);
	const mu_uint8 *alpha = reader.GetPointer(); reader.Skip(TerrainSize * TerrainSize);

	const bgfx::Memory *mem = bgfx::alloc(TerrainSize * TerrainSize * sizeof(MappingFormat));
	MappingFormat *mapping = reinterpret_cast<MappingFormat *>(mem->data);

	for (mu_uint32 index = 0; index < TerrainSize * TerrainSize; ++index)
	{
		mapping[index] = MappingFormat(
			mapping1[index],
			mapping2[index],
			mapping1[index] == 255
			? 255
			: mapping2[index] == 255
			? 0
			: alpha[index],
			0
		);
	}

	MappingTexture = bgfx::createTexture2D(TerrainSize, TerrainSize, false, 1, bgfx::TextureFormat::RGBA8U, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, mem);
	MappingSampler = bgfx::createUniform("s_mappingTexture", bgfx::UniformType::Sampler);

	return true;
}

const mu_boolean NTerrain::LoadAttributes(mu_utf8string path)
{
	NormalizePath(path);

	constexpr mu_uint32 AttributeV1Size = 65540;
	constexpr mu_uint32 AttributeV2Size = 131076;

	auto ext = path.substr(path.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
	{
		mu_error("heightmap not found ({})", path);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));

	// Well, what a shitty if
	if (
		fileLength != static_cast<mu_int64>(AttributeV1Size) &&
		fileLength != static_cast<mu_int64>(AttributeV2Size)
		)
	{
		mu_error("invalid attributes size ({})", path);
		return false;
	}

	const mu_boolean isExtended = fileLength == static_cast<mu_int64>(AttributeV2Size);

	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	XorDecrypt(buffer.get(), buffer.get(), static_cast<mu_uint32>(fileLength));
	BuxConvert(buffer.get(), static_cast<mu_uint32>(fileLength));

	NBinaryReader reader(buffer.get(), static_cast<mu_uint32>(fileLength));
	mu_uint8 version = reader.Read<mu_uint8>();
	mu_int32 map = static_cast<mu_int32>(reader.Read<mu_uint8>());
	mu_uint32 width = static_cast<mu_uint32>(reader.Read<mu_uint8>());
	mu_uint32 height = static_cast<mu_uint32>(reader.Read<mu_uint8>());

	if (version != 0 || width != 255 || height != 255)
	{
		mu_error("invalid attributes values ({})", path);
		return false;
	}
	
	if (!TerrainAttributes) TerrainAttributes.reset(new_nothrow TerrainAttribute::Type[TerrainSize * TerrainSize]);
	if (isExtended)
	{
		constexpr mu_uint32 AttributesSize = TerrainSize * TerrainSize * sizeof(mu_uint16);
		mu_memcpy(TerrainAttributes.get(), reader.GetPointer(), AttributesSize);
		reader.Skip(AttributesSize);
	}
	else
	{
		constexpr mu_uint32 AttributesSize = TerrainSize * TerrainSize;
		const mu_uint8 *attributes = reader.GetPointer(); reader.Skip(AttributesSize);
		for (mu_uint32 index = 0; index < AttributesSize; ++index)
		{
			TerrainAttributes[index] = static_cast<mu_uint16>(attributes[index]);
		}
	}

	AttributesTexture = bgfx::createTexture2D(TerrainSize, TerrainSize, false, 1, TerrainAttribute::Format, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT);
	AttributesSampler = bgfx::createUniform("s_attributesTexture", bgfx::UniformType::Sampler);

	const bgfx::Memory *mem = bgfx::makeRef(TerrainAttributes.get(), TerrainSize * TerrainSize * sizeof(TerrainAttribute::Type));
	bgfx::updateTexture2D(AttributesTexture, 0, 0, 0, 0, TerrainSize, TerrainSize, mem);

	return true;
}

const mu_boolean NTerrain::PrepareSettings(const mu_utf8string path, const nlohmann::json document)
{
	const auto water = document["water"];
	if (water.is_object() == false)
	{
		mu_error("terrain.json malformed ({})", path + "terrain.json");
		return false;
	}

	WaterModulus = water["mod"].get<mu_float>();
	WaterMultiplier = water["mul"].get<mu_float>();

	const auto wind = document["wind"];
	if (wind.is_object() == false)
	{
		mu_error("terrain.json malformed ({})", path + "terrain.json");
		return false;
	}

	WindScale = wind["scale"].get<mu_float>();
	WindModulus = wind["mod"].get<mu_float>();
	WindMultiplier = wind["mul"].get<mu_float>();

	SettingsUniform = bgfx::createUniform("u_terrainSettings", bgfx::UniformType::Vec4, 1);

	return true;
}

void NTerrain::Reset()
{
	// restore primary light per update frame
	mu_memcpy(TerrainPrimaryLight.get(), TerrainLight.get(), sizeof(glm::vec3) * TerrainSize * TerrainSize);
}

void NTerrain::ConfigureUniforms()
{
	Settings.WaterMove = glm::mod(MUState::GetWorldTime(), WaterModulus) * WaterMultiplier;
	Settings.WindScale = 10.0f;
	Settings.WindSpeed = glm::mod(MUState::GetWorldTime(), WindModulus) * WindMultiplier;
	Settings.Dummy = 0.0f;

	bgfx::setUniform(SettingsUniform, &Settings);
}

void NTerrain::Update()
{
	const bgfx::Memory *mem = bgfx::makeRef(LightmapMemory.get(), sizeof(uint8_t) * 4 * TerrainSize * TerrainSize);
	bgfx::updateTexture2D(LightmapTexture, 0, 0, 0, 0, TerrainSize, TerrainSize, mem);
	mem = bgfx::makeRef(NormalMemory.get(), sizeof(mu_uint16) * 4 * TerrainSize * TerrainSize);
	bgfx::updateTexture2D(NormalTexture, 0, 0, 0, 0, TerrainSize, TerrainSize, mem);
	mem = bgfx::makeRef(TerrainAttributes.get(), TerrainSize * TerrainSize * sizeof(TerrainAttribute::Type));
	bgfx::updateTexture2D(AttributesTexture, 0, 0, 0, 0, TerrainSize, TerrainSize, mem);
}

void NTerrain::Render()
{
	bgfx::setVertexBuffer(0, VertexBuffer);
	bgfx::setIndexBuffer(IndexBuffer);
	bgfx::setTexture(0, HeightmapSampler, HeightmapTexture);
	bgfx::setTexture(1, LightmapSampler, LightmapTexture);
	bgfx::setTexture(2, NormalSampler, NormalTexture);
	bgfx::setTexture(3, MappingSampler, MappingTexture);
	bgfx::setTexture(4, TexturesSampler, Textures);
	bgfx::setTexture(5, UVSampler, UVTexture);
	bgfx::setTexture(6, AttributesSampler, AttributesTexture);
	bgfx::submit(0, Program);
}

const bgfx::TextureHandle NTerrain::GetLightmapTexture() const
{
	return LightmapTexture;
}

const bgfx::UniformHandle NTerrain::GetLightmapSampler() const
{
	return LightmapSampler;
}

const mu_float NTerrain::GetHeight(const mu_uint32 x, const mu_uint32 y)
{
	return TerrainHeight[GetTerrainMaskIndex(x, y)];
}

const glm::vec3 NTerrain::GetLight(const mu_uint32 x, const mu_uint32 y)
{
	return TerrainLight[GetTerrainMaskIndex(x, y)];
}

const glm::vec3 NTerrain::GetPrimaryLight(const mu_uint32 x, const mu_uint32 y)
{
	return TerrainPrimaryLight[GetTerrainMaskIndex(x, y)];
}

const glm::vec3 NTerrain::GetNormal(const mu_uint32 x, const mu_uint32 y)
{
	return TerrainNormal[GetTerrainMaskIndex(x, y)];
}

const TerrainAttribute::Type NTerrain::GetAttribute(const mu_uint32 x, const mu_uint32 y)
{
	return TerrainAttributes[GetTerrainMaskIndex(x, y)];
}