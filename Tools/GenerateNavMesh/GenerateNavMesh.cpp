// GenerateNavMesh.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "shared_binaryreader.h"
#include "Recast.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include <iostream>
#include <nlohmann/fifo_map.hpp>

constexpr mu_uint32 TerrainSizeMask = 255;
constexpr mu_uint32 TerrainSize = 256;
constexpr mu_uint32 MaxTerrainSize = TerrainSize * TerrainSize;

namespace TerrainAttribute
{
	typedef mu_uint16 Type;
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

NEXTMU_INLINE void XorDecrypt(mu_uint8 *dest, const mu_uint8 *src, mu_uint32 size)
{
	static const std::array<mu_uint8, 16> XorKey = { { 0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27, 0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2 } };
	mu_uint16 key = 0x5E;
	for (mu_uint32 index = 0; index < size; ++index, ++src, ++dest)
	{
		const mu_uint8 s = *src;
		*dest = (s ^ XorKey[index % XorKey.size()]) - static_cast<mu_uint8>(key);
		key = static_cast<mu_uint8>(static_cast<mu_uint32>(s) + 0x3D);
	}
}

NEXTMU_INLINE void BuxConvert(mu_uint8 *buffer, mu_uint32 size)
{
	static const std::array<mu_uint8, 3> BuxCode = { 0xfc,0xcf,0xab };
	for (mu_uint32 index = 0; index < size; index++) {
		buffer[index] ^= BuxCode[index % static_cast<mu_uint32>(BuxCode.size())];
	}
}

const mu_boolean LoadTerrainAttributes(mu_utf8string filename, std::unique_ptr<TerrainAttribute::Type[]> &terrainAttributes)
{
	NormalizePath(filename);

	constexpr mu_uint32 AttributeV1Size = 65540;
	constexpr mu_uint32 AttributeV2Size = 131076;

	auto ext = filename.substr(filename.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		mu_error("heightmap not found ({})", filename);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));

	// Well, what a shitty if
	if (
		fileLength != static_cast<mu_int64>(AttributeV1Size) &&
		fileLength != static_cast<mu_int64>(AttributeV2Size)
		)
	{
		mu_error("invalid attributes size ({})", filename);
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
		mu_error("invalid attributes values ({})", filename);
		return false;
	}

	if (!terrainAttributes) terrainAttributes.reset(new_nothrow TerrainAttribute::Type[TerrainSize * TerrainSize]);
	if (isExtended)
	{
		constexpr mu_uint32 AttributesSize = TerrainSize * TerrainSize * sizeof(mu_uint16);
		mu_memcpy(terrainAttributes.get(), reader.GetPointer(), AttributesSize);
		reader.Skip(AttributesSize);
	}
	else
	{
		constexpr mu_uint32 AttributesSize = TerrainSize * TerrainSize;
		const mu_uint8 *attributes = reader.GetPointer(); reader.Skip(AttributesSize);
		for (mu_uint32 index = 0; index < AttributesSize; ++index)
		{
			terrainAttributes[index] = static_cast<mu_uint16>(attributes[index]);
		}
	}

	return true;
}

template<class K, class V, class dummy_compare, class A>
using wafifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using ujson = nlohmann::basic_json<wafifo_map>;

const mu_boolean GenerateTerrainNavMesh(const mu_utf8string filename)
{
	const mu_utf8string path = filename.substr(0, filename.find_last_of('\\') + 1);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		std::cout << "settings configuration missing, using default" << std::endl;
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_char[]> buffer(new_nothrow mu_char[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);
	fp = nullptr;

	const mu_utf8string inputBuffer = JsonStripComments(buffer.get(), static_cast<mu_uint32>(fileLength));
	buffer.reset();

	auto document = ujson::parse(inputBuffer.c_str());
	if (document.is_discarded() == true)
	{
		return false;
	}

	std::unique_ptr<TerrainAttribute::Type[]> terrainAttributes;
	const auto attributes = document["attributes"].get<mu_utf8string>();
	if (LoadTerrainAttributes(path + attributes, terrainAttributes) == false)
	{
		mu_error("failed to load terrain attributes ({})", path + attributes);
		return false;
	}

	rcContext context;
	auto heightfieldDeleter = [](rcHeightfield *resource) { rcFreeHeightField(resource); };
	std::unique_ptr<rcHeightfield, decltype(heightfieldDeleter)> heightfield(rcAllocHeightfield(), heightfieldDeleter);
	if (heightfield == nullptr)
	{
		return false;
	}

	const mu_float CellSize = 0.1f;
	const mu_float AgentHeight = 0.5f;
	const mu_float AgentRadius = 0.18f;
	const mu_float AgentMaxClimb = 0.9f;
	const mu_float CellHeight = 0.1f;
	const mu_int32 WalkableHeight = (mu_int32)glm::ceil((AgentHeight / CellHeight) + 0.000001f);
	const mu_int32 WalkableClimb = (mu_int32)glm::floor(AgentMaxClimb / CellHeight);
	const mu_int32 WalkableRadius = (mu_int32)glm::ceil((AgentRadius / CellSize) + 0.000001f);
	const mu_int32 BorderSize = WalkableRadius + 3;
	const mu_int32 MinRegionArea = 64;
	const mu_int32 MergeRegionArea = 400;
	const mu_float MaxContourError = 0.8f; // Max Edge Error
	const mu_int32 MaxEdgeLen = (mu_int32)(12.0f / CellSize);
	const mu_float SampleDist = CellSize * 6.0f;
	const mu_float MaxSampleError = CellHeight * 1.0f;
	const mu_int32 TerrainTriangleCount = TerrainSize * TerrainSize * 2;
	const mu_int32 TerrainVerticesCount = TerrainSize * TerrainSize * 4;

	std::unique_ptr<glm::vec3[]> vertices(new (std::nothrow) glm::vec3[TerrainVerticesCount]);
	if (!vertices)
	{
		return false;
	}

	std::unique_ptr<mu_int32[]> triangles(new (std::nothrow) mu_int32[TerrainTriangleCount * 3]);
	if (!triangles)
	{
		return false;
	}

	mu_int32 verticesCount = 0;
	mu_int32 trianglesCount = 0;
	glm::vec3 *verticesPtr = vertices.get();
	mu_int32 *trianglesPtr = triangles.get();
	for (mu_int32 y = 0; y < (mu_int32)TerrainSize; ++y)
	{
		for (mu_int32 x = 0; x < (mu_int32)TerrainSize; ++x)
		{
			const mu_uint32 src = (y & (mu_int32)TerrainSizeMask) * (mu_int32)TerrainSize + (x & (mu_int32)TerrainSizeMask);
			const mu_uint16 attr = terrainAttributes[src] & ~(TerrainAttribute::SafeZone | TerrainAttribute::Character | TerrainAttribute::Action | TerrainAttribute::Height);
			if (attr != 0) continue;

			const mu_float xf = static_cast<mu_float>(x);
			const mu_float yf = static_cast<mu_float>(y);

			glm::vec3 v4((xf), 0.0f, -(yf));
			glm::vec3 v1((xf + 1.0f), 0.0f, -(yf));
			glm::vec3 v2((xf + 1.0f), 0.0f, -(yf + 1.0f));
			glm::vec3 v3((xf), 0.0f, -(yf + 1.0f));

			const mu_int32 vertexIndex = verticesCount;
			glm::vec3 *v = &verticesPtr[verticesCount];
			v[0] = v1;
			v[1] = v2;
			v[2] = v3;
			v[3] = v4;

			mu_int32 *t = &trianglesPtr[trianglesCount * 3];
			t[0] = vertexIndex + 0;
			t[1] = vertexIndex + 1;
			t[2] = vertexIndex + 2;
			t[3] = vertexIndex + 0;
			t[4] = vertexIndex + 2;
			t[5] = vertexIndex + 3;

			verticesCount += 4;
			trianglesCount += 2;
		}
	}

	mu_float bmin[3];
	mu_float bmax[3];
	mu_int32 width, height;
	rcCalcBounds(reinterpret_cast<mu_float *>(vertices.get()), verticesCount, bmin, bmax);
	rcCalcGridSize(bmin, bmax, CellSize, &width, &height);

	if (
		rcCreateHeightfield(
			&context,
			*heightfield,
			width,
			height,
			bmin, bmax,
			CellSize,
			CellHeight
		) == false
		)
	{
		return false;
	}

	std::unique_ptr<mu_uint8[]> trianglesArea(new (std::nothrow) mu_uint8[trianglesCount]);
	if (!trianglesArea)
	{
		return false;
	}

	mu_memset(trianglesArea.get(), RC_WALKABLE_AREA, trianglesCount * sizeof(mu_uint8));

	if (
		rcRasterizeTriangles(
			&context,
			reinterpret_cast<mu_float *>(vertices.get()),
			verticesCount,
			triangles.get(),
			trianglesArea.get(),
			trianglesCount,
			*heightfield,
			WalkableClimb
		) == false
		)
	{
		return false;
	}

	rcFilterLowHangingWalkableObstacles(&context, WalkableClimb, *heightfield);
	rcFilterLedgeSpans(&context, WalkableHeight, WalkableClimb, *heightfield);
	rcFilterWalkableLowHeightSpans(&context, WalkableHeight, *heightfield);

	auto cheightfieldDeleter = [](rcCompactHeightfield *resource) { rcFreeCompactHeightfield(resource); };
	std::unique_ptr<rcCompactHeightfield, decltype(cheightfieldDeleter)> cheightfield(rcAllocCompactHeightfield(), cheightfieldDeleter);
	if (cheightfield == nullptr)
	{
		return false;
	}

	if (
		rcBuildCompactHeightfield(
			&context,
			WalkableHeight,
			WalkableClimb,
			*heightfield,
			*cheightfield
		) == false
		)
	{
		return false;
	}

	heightfield.reset();

	if (
		rcErodeWalkableArea(
			&context,
			WalkableRadius,
			*cheightfield
		) == false
		)
	{
		return false;
	}

	if (!rcBuildDistanceField(&context, *cheightfield))
	{
		return false;
	}

	if (!rcBuildRegions(&context, *cheightfield, 0, MinRegionArea, MergeRegionArea))
	{
		return false;
	}

	auto contourSetDeleter = [](rcContourSet *resource) { rcFreeContourSet(resource); };
	std::unique_ptr<rcContourSet, decltype(contourSetDeleter)> contourSet(rcAllocContourSet(), contourSetDeleter);
	if (contourSet == nullptr)
	{
		return false;
	}

	if (
		rcBuildContours(
			&context,
			*cheightfield,
			MaxContourError,
			MaxEdgeLen,
			*contourSet
		) == false
		)
	{
		return false;
	}

	auto polymeshDeleter = [](rcPolyMesh *resource) { rcFreePolyMesh(resource); };
	std::unique_ptr<rcPolyMesh, decltype(polymeshDeleter)> polymesh(rcAllocPolyMesh(), polymeshDeleter);
	if (polymesh == nullptr)
	{
		return false;
	}

	if (
		rcBuildPolyMesh(
			&context,
			*contourSet,
			6,
			*polymesh
		) == false
		)
	{
		return false;
	}

	auto polymeshDetailDeleter = [](rcPolyMeshDetail *resource) { rcFreePolyMeshDetail(resource); };
	std::unique_ptr<rcPolyMeshDetail, decltype(polymeshDetailDeleter)> polymeshDetail(rcAllocPolyMeshDetail(), polymeshDetailDeleter);
	if (polymeshDetail == nullptr)
	{
		return false;
	}

	if (
		rcBuildPolyMeshDetail(
			&context,
			*polymesh,
			*cheightfield,
			SampleDist,
			MaxSampleError,
			*polymeshDetail
		) == false
		)
	{
		return false;
	}

	contourSet.reset();
	cheightfield.reset();

	enum : mu_uint8
	{
		POLYAREA_GROUND,
	};

	enum : mu_uint8
	{
		POLYFLAGS_WALK = 1 << 0,
	};

	for (mu_int32 n = 0; n < polymesh->npolys; ++n)
	{
		polymesh->areas[n] = POLYAREA_GROUND;
		if (polymesh->areas[n] == POLYAREA_GROUND)
		{
			polymesh->flags[n] = POLYFLAGS_WALK;
		}
	}

	dtNavMeshCreateParams params;
	mu_memset(&params, 0, sizeof(params));
	params.verts = polymesh->verts;
	params.vertCount = polymesh->nverts;
	params.polys = polymesh->polys;
	params.polyAreas = polymesh->areas;
	params.polyFlags = polymesh->flags;
	params.polyCount = polymesh->npolys;
	params.nvp = polymesh->nvp;
	params.detailMeshes = polymeshDetail->meshes;
	params.detailVerts = polymeshDetail->verts;
	params.detailVertsCount = polymeshDetail->nverts;
	params.detailTris = polymeshDetail->tris;
	params.detailTriCount = polymeshDetail->ntris;
	params.offMeshConVerts = nullptr;
	params.offMeshConRad = nullptr;
	params.offMeshConDir = nullptr;
	params.offMeshConAreas = nullptr;
	params.offMeshConFlags = nullptr;
	params.offMeshConUserID = nullptr;
	params.offMeshConCount = 0;
	params.walkableHeight = AgentHeight;
	params.walkableRadius = AgentRadius;
	params.walkableClimb = AgentMaxClimb;
	rcVcopy(params.bmin, polymesh->bmin);
	rcVcopy(params.bmax, polymesh->bmax);
	params.cs = CellSize;
	params.ch = CellHeight;
	params.buildBvTree = true;

	mu_uint8 *navData = 0;
	mu_int32 navDataSize = 0;
	if (
		dtCreateNavMeshData(
			&params,
			&navData,
			&navDataSize
		) == false
		)
	{
		return false;
	}

	auto navMeshDeleter = [](dtNavMesh *resource) { dtFreeNavMesh(resource); };
	std::unique_ptr<dtNavMesh, decltype(navMeshDeleter)> navMesh(dtAllocNavMesh(), navMeshDeleter);
	if (navMesh == nullptr)
	{
		return false;
	}

	dtStatus status;
	status = navMesh->init(navData, navDataSize, 0);
	if (dtStatusFailed(status))
	{
		return false;
	}

	auto navMeshQueryDeleter = [](dtNavMeshQuery *resource) { dtFreeNavMeshQuery(resource); };
	std::unique_ptr<dtNavMeshQuery, decltype(navMeshQueryDeleter)> navMeshQuery(dtAllocNavMeshQuery(), navMeshQueryDeleter);
	if (navMeshQuery == nullptr)
	{
		return false;
	}

	status = navMeshQuery->init(navMesh.get(), 2048);
	if (dtStatusFailed(status))
	{
		return false;
	}

	const mu_utf8string navFilename = path + "terrain.nav";
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, navFilename, "wb") == false)
	{
		std::cout << "failed to write nav data file" << std::endl;
		return false;
	}

	SDL_RWwrite(fp, navData, 1, navDataSize);
	SDL_RWclose(fp);
	fp = nullptr;

	document["nav_mesh"] = "terrain.nav";

	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "wb") == false)
	{
		std::cout << "failed to write json" << std::endl;
		return false;
	}

	const mu_utf8string jsonData = document.dump(1, '\t');
	SDL_RWwrite(fp, jsonData.data(), jsonData.size(), 1);
	SDL_RWclose(fp);

	return true;
}

int main(int argc, char *argv[])
{
	if (SDL_Init(0) < 0)
	{
		std::cout << "failed to initialize SDL" << std::endl;
		return 0;
	}

	if (argc != 2)
	{
		std::cout << "encterrain.obj missing" << std::endl;
		return 0;
	}

	const mu_utf8string filename = argv[1];
	GenerateTerrainNavMesh(filename);

	SDL_Quit();

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
