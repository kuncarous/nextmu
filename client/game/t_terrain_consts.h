#ifndef __T_TERRAIN_CONSTS_H__
#define __T_TERRAIN_CONSTS_H__

#pragma once

constexpr mu_float TerrainScale = 100.0f;
constexpr mu_float TerrainScaleInv = 1.0f / TerrainScale;
constexpr mu_uint32 TerrainSize = 256u; // MU has a fixed terrain size of 256x256
constexpr mu_uint32 TerrainMask = TerrainSize - 1;
constexpr mu_float TerrainSizeInv = 1.0f / static_cast<mu_float>(TerrainSize);

#endif