#include "stdafx.h"
#include "mu_controllerstate.h"

namespace MUControllerState
{
	entt::entity Character = entt::null;

	void SetCharacter(const entt::entity entity)
	{
		Character = entity;
	}

	const entt::entity GetEntity()
	{
		return Character;
	}
};