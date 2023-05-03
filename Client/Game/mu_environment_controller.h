#ifndef __MU_ENVIRONMENT_CONTROLLER_H__
#define __MU_ENVIRONMENT_CONTROLLER_H__

#pragma once

#include "mu_camera.h"

typedef std::unique_ptr<NCamera> NCameraPtr;

class NEnvironment;
class NController
{
public:
	NController(const NEnvironment *environment);
	~NController();

	void Update();

private:
	void Configure();

public:
	NCamera *GetCamera() const
	{
		return Camera.get();
	}

	void SetCharacter(const entt::entity entity)
	{
		Character = entity;
		Configure();
	}

	const entt::entity GetCharacter() const
	{
		return Character;
	}

private:
	const NEnvironment *Environment;
	NCameraPtr Camera;
	entt::entity Character;
};

#endif