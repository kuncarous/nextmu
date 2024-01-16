#include "mu_precompiled.h"
#include "t_terrain_cullingtree.h"
#include "mu_renderstate.h"
#include "mu_camera.h"

const mu_boolean NTerrainCullingTree::Initialize(const mu_float *heightmap)
{
	mu_uint32 size = TerrainSize;
	mu_uint32 tsize = size;
	mu_uint32 blockSize = size * size;
	mu_uint32 offset = CullingTreeBlocks - size * size;

	do
	{
		if (size == TerrainSize)
		{
			for (mu_uint32 index = 0; index < blockSize; ++index)
			{
				const mu_uint32 x = index % TerrainSize;
				const mu_uint32 y = index / TerrainSize;

				auto &block = Blocks[offset + index];
				block.SX = x;
				block.EX = x + 1u;
				block.SY = y;
				block.EY = y + 1u;

				const mu_float ZX1Y1 = heightmap[GetTerrainMaskIndex(x, y)];
				const mu_float ZX2Y1 = heightmap[GetTerrainMaskIndex(x+1, y)];
				const mu_float ZX1Y2 = heightmap[GetTerrainMaskIndex(x, y+1)];
				const mu_float ZX2Y2 = heightmap[GetTerrainMaskIndex(x+1, y+1)];

				block.SZ = glm::min(ZX1Y1, glm::min(ZX2Y1, glm::min(ZX1Y2, ZX2Y2)));
				block.EZ = glm::max(ZX1Y1, glm::max(ZX2Y1, glm::max(ZX1Y2, ZX2Y2)));
				if (block.EZ - block.SZ <= glm::epsilon<mu_float>()) {
					block.SZ -= 1.0f;
					block.EZ += 1.0f;
				}
			}
		}
		else
		{
			const mu_uint32 noffset = offset + blockSize;
			for (mu_uint32 index = 0; index < blockSize; ++index)
			{
				const mu_uint32 x = index % size;
				const mu_uint32 y = index / size;
				const mu_uint32 mx = x * CullingTreeSize;
				const mu_uint32 my = y * CullingTreeSize;

				auto &block = Blocks[offset + index];
				block.SX = std::numeric_limits<mu_uint16>::max();
				block.EX = std::numeric_limits<mu_uint16>::min();
				block.SY = std::numeric_limits<mu_uint16>::max();
				block.EY = std::numeric_limits<mu_uint16>::min();
				block.SZ = std::numeric_limits<mu_float>::max();
				block.EZ = std::numeric_limits<mu_float>::min();

				for (mu_uint32 by = 0; by < CullingTreeSize; ++by)
				{
					const mu_uint32 boffset = noffset + (my + by) * tsize;
					for (mu_uint32 bx = 0; bx < CullingTreeSize; ++bx)
					{
						auto &b = Blocks[boffset + mx + bx];
						if (b.SX < block.SX) block.SX = b.SX;
						if (b.EX > block.EX) block.EX = b.EX;
						if (b.SY < block.SY) block.SY = b.SY;
						if (b.EY > block.EY) block.EY = b.EY;
						if (b.SZ < block.SZ) block.SZ = b.SZ;
						if (b.EZ > block.EZ) block.EZ = b.EZ;
					}
				}
			}
		}

		size /= CullingTreeSize;
		tsize = size * CullingTreeSize;
		blockSize = size * size;
		offset -= blockSize;
	} while (size >= StartCullingTreeDepth);

	return true;
}

void NTerrainCullingTree::GenerateRenderRanges(NTerrainRenderSettings &settings)
{
	//auto startTimer = std::chrono::high_resolution_clock::now();
	const auto camera = MURenderState::GetCamera();
	const auto frustum = camera->GetFrustum();

	for (mu_uint32 ry = 0u; ry < StartCullingTreeDepth; ++ry)
	{
		for (mu_uint32 rx = 0u; rx < StartCullingTreeDepth; ++rx)
		{
			TraverseBlocks(frustum, StartCullingTreeDepth, 0u, rx, ry, settings);
		}
	}
	//auto endTimer = std::chrono::high_resolution_clock::now();
	//auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(endTimer - startTimer);
	//mu_info("[DEBUG] GenerateRenderRanges : {}ms", diff.count());
}

void NTerrainCullingTree::TraverseBlocks(
	const Diligent::ViewFrustumExt *frustum,
	const mu_uint32 size,
	const mu_uint32 offset,
	const mu_uint32 rx,
	const mu_uint32 ry,
	NTerrainRenderSettings &settings
)
{
	const auto roffset = offset + ry * size + rx;
	const auto &block = Blocks[roffset];
	Diligent::BoundBox box;
	box.Min = Diligent::float3(static_cast<mu_float>(block.SX) * TerrainScale - TerrainScale * 0.5f, block.SZ - 2.0f, static_cast<mu_float>(block.SY) * TerrainScale - TerrainScale * 0.5f);
	box.Max = Diligent::float3(static_cast<mu_float>(block.EX) * TerrainScale + TerrainScale * 0.5f, block.EZ + 2.0f, static_cast<mu_float>(block.EY) * TerrainScale + TerrainScale * 0.5f);
	const auto visibility = Diligent::GetBoxVisibility(
		*frustum,
		box
	);
	if (visibility == Diligent::BoxVisibility::Invisible) return;

	if (size >= TerrainSize)
	{
		auto &render = settings.Ranges[block.SY];
		auto [iter, inserted] = settings.Lines.insert(block.SY);
		if (inserted)
		{
			render.Start = (block.SY * TerrainSize + block.SX) * 6u;
			render.End = render.Start + 6u;
		}
		else
		{
			const mu_uint32 vstart = (block.SY * TerrainSize + block.SX) * 6u;
			const mu_uint32 vend = (block.SY * TerrainSize + block.SX) * 6u + 6u;
			if (vstart < render.Start) render.Start = vstart;
			if (vend > render.End) render.End = vend;
		}
		return;
	}

	if (visibility == Diligent::BoxVisibility::Intersecting)
	{
		const mu_uint32 msize = size * CullingTreeSize;
		const mu_uint32 moffset = offset + size * size;
		const mu_uint32 mx = rx * CullingTreeSize;
		const mu_uint32 my = ry * CullingTreeSize;

		for (mu_uint32 by = 0; by < CullingTreeSize; ++by)
		{
			const mu_uint32 boffset = roffset + (my + by) * size;
			for (mu_uint32 bx = 0; bx < CullingTreeSize; ++bx)
			{
				TraverseBlocks(frustum, msize, moffset, mx + bx, my + by, settings);
			}
		}
	}
	else
	{
		for (mu_uint32 y = block.SY; y < block.EY; ++y)
		{
			auto [iter, inserted] = settings.Lines.insert(y);
			const mu_uint32 vstart = (y * TerrainSize + block.SX) * 6u;
			const mu_uint32 vend = (y * TerrainSize + block.EX) * 6u;

			auto &render = settings.Ranges[y];
			if (inserted)
			{
				render.Start = vstart;
				render.End = vend;
			}
			else
			{
				if (vstart < render.Start) render.Start = vstart;
				if (vend > render.End) render.End = vend;
			}
		}
	}
}