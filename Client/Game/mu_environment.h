#ifndef __MU_ENVIRONMENT_H__
#define __MU_ENVIRONMENT_H__

#pragma once

#include "mu_environment_object.h"
#include "mu_environment_character.h"
#include "mu_entity.h"

class NTerrain;

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

private: // Objects
	const mu_boolean LoadObjects(mu_utf8string filename, const std::map<mu_uint32, NModel *> models);

public: // Objects
	void ClearObjects();
	const entt::entity AddObject(
		const MUObject::Settings object
	);
	void RemoveObject(const entt::entity entity);

public: // Characters
	void ClearCharacters();
	const entt::entity AddOrFindCharacter(
		const MUCharacter::Settings character
	);
	void RemoveCharacter(const entt::entity entity);

public:
	const NTerrain *GetTerrain() const
	{
		return Terrain.get();
	}

private:
	NTerrainPtr Terrain;
	std::vector<NModelPtr> Models;
	entt::registry Objects;
	entt::registry Characters;
	std::map<mu_key, entt::entity> CharactersMap;
};

#endif