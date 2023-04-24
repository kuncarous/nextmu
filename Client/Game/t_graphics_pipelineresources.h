#ifndef __T_GRAPHICS_PIPELINERESOURCES_H__
#define __T_GRAPHICS_PIPELINERESOURCES_H__

#pragma once

struct NPipelineResource
{
	std::vector<Diligent::ShaderResourceVariableDesc> Variables;
	std::vector<Diligent::ImmutableSamplerDesc> ImmutableSamplers;
};

void CreatePipelineResources();
const NPipelineResource *GetPipelineResource(const mu_utf8string resourceId);

#endif