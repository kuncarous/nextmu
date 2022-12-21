#ifndef __MU_ENTITY_LIGHT_H__
#define __MU_ENTITY_LIGHT_H__

#pragma once

enum class EntityLightMode
{
	Terrain,
	Fixed,
	SinWorldTime,
};

NEXTMU_INLINE const EntityLightMode LightModeFromString(const mu_utf8string mode)
{
	if (mode == "terrain") return EntityLightMode::Terrain;
	if (mode == "fixed") return EntityLightMode::Fixed;
	if (mode == "sinworld") return EntityLightMode::SinWorldTime;
	return EntityLightMode::Terrain;
}

#endif