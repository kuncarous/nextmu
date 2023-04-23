#ifndef __T_GRAPHICS_PIPELINES_H__
#define __T_GRAPHICS_PIPELINES_H__

#pragma once

struct NFixedPipelineState;
struct NDynamicPipelineState;

typedef mu_uint32 NPipelineStateId;
typedef mu_uint32 NPipelineBlendHash;
struct NPipelineState
{
	NPipelineStateId Id;
	NPipelineBlendHash BlendHash;
	Diligent::RefCntAutoPtr<Diligent::IPipelineState> Pipeline;
};

void DestroyPipelines();
NPipelineState *GetPipelineState(const NFixedPipelineState &fixedState, const NDynamicPipelineState &dynamicState);

#endif