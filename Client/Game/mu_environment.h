#ifndef __MU_ENVIRONMENT_H__
#define __MU_ENVIRONMENT_H__

#pragma once

#include "mu_environment_objects.h"
#include "mu_environment_characters.h"
#include "mu_environment_particles.h"
#include "mu_environment_joints.h"
#include "mu_entity.h"
#include "t_threading_helper.h"

class NTerrain;

typedef std::unique_ptr<Diligent::ShadowMapManager> NShadowMapPtr;
typedef std::unique_ptr<NTerrain> NTerrainPtr;
typedef std::unique_ptr<NModel> NModelPtr;
typedef std::unique_ptr<NObjects> NObjectsPtr;
typedef std::unique_ptr<NCharacters> NCharactersPtr;
typedef std::unique_ptr<NParticles> NParticlesPtr;
typedef std::unique_ptr<NJoints> NJointsPtr;

class NEnvironment
{
public:
	const mu_boolean Initialize();
	void Destroy();

	const mu_boolean CreateShadowMap();

	void Reset(const mu_boolean forceReset = false);
	void Update();
	void Render();
	void CalculateLight(
		const NEntity::NPosition &position,
		const NEntity::NLight &settings,
		NEntity::NRenderState &renderState
	) const;

	const mu_boolean LoadTerrain(mu_utf8string path);

private:
	const mu_boolean LoadObjects(mu_utf8string filename, const std::map<mu_uint32, NModel *> models);

public:
	NObjects *GetObjects() const
	{
		return Objects.get();
	}

	NCharacters *GetCharacters() const
	{
		return Characters.get();
	}

	NParticles *GetParticles() const
	{
		return Particles.get();
	}

	NJoints *GetJoints() const
	{
		return Joints.get();
	}

	NTerrain *GetTerrain() const
	{
		return Terrain.get();
	}

private:
	NRenderSettings RenderSettings;
	Diligent::LightAttribs LightAttribs;

	Diligent::float3 LightDirection = Diligent::float3(-1.3f, 0.0f, -2.0f);//Diligent::float3(-0.554699242f, -0.0599640049f, -0.829887390f);
	NResourceId ShadowResourceId = NInvalidUInt32;
	NShadowMapPtr ShadowMap;
	std::vector<Diligent::ViewFrustumExt> ShadowFrustums;

	NTerrainPtr Terrain;
	std::vector<NModelPtr> Models;
	NObjectsPtr Objects;
	NCharactersPtr Characters;
	NParticlesPtr Particles;
	NJointsPtr Joints;
};

#endif