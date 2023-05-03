#ifndef __MU_ENVIRONMENT_OBJECTS_H__
#define __MU_ENVIRONMENT_OBJECTS_H__

#pragma once

#include "t_object_structs.h"

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

public:
	entt::registry &GetRegistry()
	{
		return Registry;
	}

private:
	const NEnvironment *Environment;
	entt::registry Registry;
};

#endif