#ifndef __MU_ENVIRONMENT_OBJECTS_H__
#define __MU_ENVIRONMENT_OBJECTS_H__

#pragma once

#include "t_object_structs.h"

class NObjects
{
public:
	const mu_boolean Initialize();
	void Destroy();

	void Update(const NRenderSettings &renderSettings);
	void Render(const NRenderSettings &renderSettings);

	void Clear();
	const entt::entity Add(
		const TObject::Settings object
	);
	void Remove(const entt::entity entity);

private:
	entt::registry Registry;
};

#endif