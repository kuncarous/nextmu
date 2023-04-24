#ifndef __T_GRAPHICS_SHADERRESOURCES_H__
#define __T_GRAPHICS_SHADERRESOURCES_H__

#pragma once

#include "t_graphics_resources.h"

typedef mu_uint32 NShaderResourcesId;
typedef mu_uint32 NShaderResourcesComponents;

struct NShaderResourcesBinding
{
	NPipelineStateId PipelineId;
	NShaderResourcesId ShaderResourceId;
	mu_boolean Initialized;
	mu_boolean ShouldTransition;
	std::vector<NResourceId> Resources;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> Binding;
};

void DestroyShaderBindings();
NShaderResourcesBinding *GetShaderBinding(NPipelineState *pipeline, const mu_uint32 numResources, NResourceId *resources);
void MergeTemporaryShaderBindings();
void ReleaseShaderResourcesByResourceId(const NResourceId id);
void ReleaseShaderResources(const NShaderResourcesBinding *binding);

#endif