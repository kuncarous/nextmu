#ifndef __T_PARTICLE_FLOWER01_V0_H__
#define __T_PARTICLE_FLOWER01_V0_H__

#pragma once

#include "t_particle_base.h"

class TParticleFlower01V0 : public TParticle::Template
{
public:
	TParticleFlower01V0();
public:
	virtual void Initialize() override;
	virtual void Create(TParticle::EnttRegistry &registry, const NParticleData &data) override;
	virtual TParticle::EnttIterator Move(TParticle::EnttRegistry &registry, TParticle::EnttView &view, TParticle::EnttIterator iter, TParticle::EnttIterator last) override;
	virtual TParticle::EnttIterator Action(TParticle::EnttRegistry &registry, TParticle::EnttView &view, TParticle::EnttIterator iter, TParticle::EnttIterator last) override;
	virtual TParticle::EnttIterator Render(TParticle::EnttRegistry &registry, TParticle::EnttView &view, TParticle::EnttIterator iter, TParticle::EnttIterator last, TParticle::NRenderBuffer &renderBuffer) override;
	virtual void RenderGroup(const TParticle::NRenderGroup &renderGroup, TParticle::NRenderBuffer &renderBuffer) override;
};

#endif