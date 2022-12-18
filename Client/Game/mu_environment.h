#ifndef __MU_ENVIRONMENT_H__
#define __MU_ENVIRONMENT_H__

#pragma once

#include "mu_environment_structs.h"

class NTerrain;
class NModel;

typedef std::unique_ptr<NTerrain> NTerrainPtr;
typedef std::unique_ptr<NModel> NModelPtr;

class NEnvironment
{
public:
	void Reset();
	void Update();
	void Render();
	void CalculateLight(
		const NEntity::Position &position,
		const NEntity::Light &settings,
		NEntity::RenderState &renderState
	);

	const mu_boolean LoadTerrain(mu_utf8string path);

private:
	const mu_boolean LoadObjects(mu_utf8string filename, const std::map<mu_uint32, NModel *> models);

public:
	void ClearObjects();
	const entt::entity AddObject(
		const MUObject::Settings object
	);
	void RemoveObject(const entt::entity entity);

public:
	const NTerrain *GetTerrain() const
	{
		return Terrain.get();
	}

private:
	NTerrainPtr Terrain;
	std::vector<NModelPtr> Models;
	entt::registry Objects;
};

#endif