#ifndef __T_GRAPHICS_PIPELINES_H__
#define __T_GRAPHICS_PIPELINES_H__

#pragma once

struct NFixedPipelineState;
struct NDynamicPipelineState;

typedef mu_uint32 NPipelineStateId;
typedef mu_uint32 NPipelineBlendHash;

struct NPipelineStateInfo
{
	mu_shader Shader;
	mu_boolean DepthWrite;
	mu_boolean BlendEnable;
	Diligent::BLEND_FACTOR SrcBlend;
	Diligent::BLEND_FACTOR DestBlend;
	NPipelineBlendHash BlendHash;
};

struct NPipelineState
{
	NPipelineStateId Id;
	NPipelineStateInfo Info;
	mu_boolean StaticInitialized = false;
	Diligent::RefCntAutoPtr<Diligent::IPipelineState> Pipeline;
};

void DestroyPipelines();
NPipelineState *GetPipelineState(const NFixedPipelineState &fixedState, const NDynamicPipelineState &dynamicState);

#endif