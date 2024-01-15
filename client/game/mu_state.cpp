#include "stdafx.h"
#include "mu_state.h"

namespace MUState
{
	mu_float WorldTime;
	mu_float ElapsedTime;
	mu_float UpdateTime;
	mu_uint32 UpdateCount;
	mu_float Luminosity;
	glm::vec3 Luminosity3;

	entt::entity HeroEntity = entt::null;

	void SetTime(const mu_float worldTime, const mu_float elapsedTime)
	{
		WorldTime = worldTime;
		ElapsedTime = elapsedTime;
		Luminosity = glm::sin(worldTime * 0.004f) * 0.15f + 0.5f;
		Luminosity3 = glm::vec3(Luminosity, Luminosity, Luminosity);
	}

	void SetUpdate(const mu_float updateTime, const mu_uint32 updateCount)
	{
		UpdateTime = updateTime;
		UpdateCount = updateCount;
	}

	const mu_float GetWorldTime()
	{
		return WorldTime;
	}

	const mu_float GetElapsedTime()
	{
		return ElapsedTime;
	}

	const mu_float GetUpdateTime()
	{
		return UpdateTime;
	}

	const mu_uint32 GetUpdateCount()
	{
		return UpdateCount;
	}

	const mu_float GetLuminosity()
	{
		return Luminosity;
	}

	glm::vec3 GetLuminosityVector3()
	{
		return Luminosity3;
	}

	void SetHero(const entt::entity entity)
	{
		HeroEntity = entity;
	}

	const entt::entity GetHero()
	{
		return HeroEntity;
	}
};