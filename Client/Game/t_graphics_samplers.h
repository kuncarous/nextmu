#ifndef __T_GRAPHICS_SAMPLERS_H__
#define __T_GRAPHICS_SAMPLERS_H__

#pragma once

#include "t_graphics_resourceid.h"

class NSampler
{
public:
	NSampler(Diligent::RefCntAutoPtr<Diligent::ISampler> sampler);

public:
	NResourceId Id;
	Diligent::RefCntAutoPtr<Diligent::ISampler> Sampler;
};

NSampler *GetTextureSampler(const Diligent::SamplerDesc &samplerDesc);

#endif