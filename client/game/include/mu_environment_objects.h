#ifndef __MU_ENVIRONMENT_OBJECTS_H__
#define __MU_ENVIRONMENT_OBJECTS_H__

#pragma once

#include "t_object_structs.h"

class NFadingGroup
{
public:
	NFadingGroup(const mu_float target = 0.2f, const mu_float speed = 0.1f) : Fading(false), Target(target), Speed(speed) {}

	mu_atomic_bool Fading;
	mu_float Target;
	mu_float Speed;
};

class NEnvironment;
class NObjects
{
public:
	NObjects(const NEnvironment *environment);

	const mu_boolean Initialize();
	void Destroy();

	void Update();
	void PreRender(const NRenderSettings &renderSettings);
	void Render(const NRenderSettings &renderSettings);

	void Clear();
	const entt::entity Add(
		const TObject::Settings object
	);
	void Remove(const entt::entity entity);

	void ClearFadingGroups();
	void AddFadingGroup(const mu_uint32 group, const mu_float target, const mu_float speed);
	NFadingGroup *GetFadingGroup(const mu_uint32 group);

public:
	entt::registry &GetRegistry()
	{
		return Registry;
	}

private:
	typedef std::map<const mu_uint32, std::unique_ptr<NFadingGroup>> NFadingGroupsMap;
	const NEnvironment *Environment;
	entt::registry Registry;
	NFadingGroupsMap FadingGroups;
};

#endif