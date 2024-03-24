#ifndef __MU_STATE_H__
#define __MU_STATE_H__

#pragma once

constexpr mu_double GameCycleTime = 1000.0 / 25.0; // 25 FPS
constexpr mu_float GameCycleDivisor = static_cast<mu_float>(1.0 / GameCycleTime);

namespace MUState
{
	void SetTime(const mu_float worldTime, const mu_float elapsedTime);
	void SetUpdate(const mu_float updateTime, const mu_uint32 updateCount);

	const mu_float GetWorldTime();
	const mu_float GetElapsedTime();
	const mu_float GetUpdateTime();
	const mu_uint32 GetUpdateCount();
	const mu_float GetLuminosity();
	glm::vec3 GetLuminosityVector3();

	void SetHero(const entt::entity entity);
	const entt::entity GetHero();
};

#endif