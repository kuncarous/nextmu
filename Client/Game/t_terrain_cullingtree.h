#ifndef __T_TERRAIN_CULLINGTREE_H__
#define __T_TERRAIN_CULLINGTREE_H__

#pragma once

#include "t_terrain_consts.h"
#include "t_terrain_structs.h"

NEXTMU_INLINE mu_uint32 PackTerrainSquare(const mu_uint32 x, const mu_uint32 y)
{
	return y << 8 | x;
}

NEXTMU_INLINE void UnpackTerrainSquare(const mu_uint32 index, mu_uint32 &x, mu_uint32 &y)
{
	x = index & 0xFF;
	y = (index >> 8) & 0xFF;
}

constexpr mu_uint32 CullingTreeSize = 4u;
constexpr mu_uint32 StartCullingTreeDepth = 16u;

constexpr mu_uint32 CalculateCullingTreeBlocks(const mu_uint32 size)
{
	if (size < StartCullingTreeDepth) return 0u;
	return size * size + CalculateCullingTreeBlocks(size / CullingTreeSize);
}

constexpr mu_uint32 CullingTreeBlocks = CalculateCullingTreeBlocks(TerrainSize);

struct NCullingTreeBlock
{
	mu_uint16 SX, SY;
	mu_uint16 EX, EY;
	mu_float SZ, EZ;
};

class NTerrainCullingTree
{
public:
	const mu_boolean Initialize(const mu_float *heightmap);
	void GenerateRenderRanges(NTerrainRenderSettings &settings);

private:
	void TraverseBlocks(
		const Diligent::ViewFrustumExt *frustum,
		const mu_uint32 size,
		const mu_uint32 offset,
		const mu_uint32 rx,
		const mu_uint32 ry,
		NTerrainRenderSettings &settings
	);

private:
	std::array<NCullingTreeBlock, CullingTreeBlocks> Blocks;
};

#endif