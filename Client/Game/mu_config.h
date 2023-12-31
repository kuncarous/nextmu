#ifndef __MU_CONFIG_H__
#define __MU_CONFIG_H__

#pragma once

namespace MUConfig
{
	const mu_boolean Initialize();
	void Destroy();

	void Save();

	const mu_boolean GetWindowMode();
	const mu_uint32 GetWindowWidth();
	const mu_uint32 GetWindowHeight();

	const mu_boolean GetEnableShadows();
	const NShadowMode GetShadowMode();
	const mu_float GetShadowFarZ();
	const mu_boolean GetShadowRightHanded();
	const mu_uint32 GetShadowCascadesCount();
	const mu_uint32 GetShadowResolution();
	const mu_float GetShadowPartitioning();
	const mu_uint32 GetShadowFilterSize();
	const mu_boolean GetShadowFilterAcrossCascades();
	const mu_boolean GetShadowBestCascadeSearch();

	const mu_boolean GetAntialiasing();
	const mu_boolean GetVerticalSync();
};

#endif