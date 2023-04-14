#include "stdafx.h"
#include "mu_environment.h"
#include "mu_state.h"
#include "mu_camera.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "mu_renderstate.h"
#include "mu_threadsmanager.h"
#include <algorithm>
#include <execution>

enum class NThreadMode {
	Single,
	Multi,
	MultiSTL,
};

#define RENDER_BBOX (0)
constexpr NThreadMode ThreadMode = NThreadMode::Multi;

const mu_boolean NEnvironment::Initialize()
{
	Objects.reset(new (std::nothrow) NObjects());
	if (!Objects || Objects->Initialize() == false)
	{
		return false;
	}

	Characters.reset(new (std::nothrow) NCharacters());
	if (!Characters || Characters->Initialize() == false)
	{
		return false;
	}

	Particles.reset(new (std::nothrow) NParticles());
	if (!Particles || Particles->Initialize() == false)
	{
		return false;
	}

	Joints.reset(new (std::nothrow) NJoints());
	if (!Joints || Joints->Initialize() == false)
	{
		return false;
	}

	return true;
}

void NEnvironment::Destroy()
{
	if (Particles)
	{
		Particles->Destroy();
		Particles.reset();
	}

	if (Joints)
	{
		Joints->Destroy();
		Joints.reset();
	}
}

void NEnvironment::Reset()
{
	const auto updateCount = MUState::GetUpdateCount();

	if (updateCount > 0)
	{
		Terrain->Reset();
	}
}

void NEnvironment::Update()
{
	const auto updateCount = MUState::GetUpdateCount();
	const auto updateTime = MUState::GetUpdateTime();

	if (updateCount > 0)
	{
		Terrain->Update();
	}

	Objects->Update();
	Characters->Update();

	Particles->Update();
	Particles->Propagate();

	Joints->Update();
	Joints->Propagate();
}

void NEnvironment::Render()
{
	Terrain->ConfigureUniforms();
	Terrain->Render();
	
	Objects->Render();
	Characters->Render();

	Particles->Render();
	Joints->Render();
}

void NEnvironment::CalculateLight(
	const NEntity::NPosition &position,
	const NEntity::NLight &light,
	NEntity::NRenderState &renderState
) const
{
	switch (light.Mode)
	{
	case EntityLightMode::Terrain:
		{
			const auto &terrain = light.Settings.Terrain;
			const glm::vec3 terrainLight = (
				terrain.PrimaryLight
				? Terrain->CalculatePrimaryLight(position.Position[0], position.Position[1])
				: Terrain->CalculateBackLight(position.Position[0], position.Position[1])
			);
			renderState.BodyLight = glm::vec4(terrain.Color + terrainLight, 1.0f);
		}
		break;

	case EntityLightMode::Fixed:
		{
			const auto &fixed = light.Settings.Fixed;
			renderState.BodyLight = glm::vec4(fixed.Color, 1.0f);
		}
		break;

	case EntityLightMode::SinWorldTime:
		{
			const auto &worldTime = light.Settings.WorldTime;
			mu_float luminosity = glm::sin(MUState::GetWorldTime() * worldTime.TimeMultiplier) * worldTime.Multiplier + worldTime.Add;
			renderState.BodyLight = glm::vec4(luminosity, luminosity, luminosity, 1.0f);
		}
		break;
	}
}