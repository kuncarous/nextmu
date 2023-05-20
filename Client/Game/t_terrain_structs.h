#ifndef __T_TERRAIN_STRUCTS_H__
#define __T_TERRAIN_STRUCTS_H__

#pragma once

#include <set>

struct NTerrainRenderRange
{
	mu_uint32 Start;
	mu_uint32 End;
};

struct NTerrainRenderSettings
{
	std::set<mu_uint8> Lines;
	std::vector<NTerrainRenderRange> Ranges;
};

#endif