#ifndef __T_GRAPHICS_SHADER_H__
#define __T_GRAPHICS_SHADER_H__

#pragma once

struct NPipelineResource;
struct NCombinedShader
{
	Diligent::InputLayoutDesc Layout;
	const NPipelineResource *Resource;
	Diligent::RefCntAutoPtr<Diligent::IShader> Vertex;
	Diligent::RefCntAutoPtr<Diligent::IShader> Pixel;
};

typedef mu_uint16 mu_shader;
constexpr mu_shader NInvalidShader = NInvalidUInt16;

mu_shader RegisterShader(NCombinedShader shader);
NCombinedShader *GetShader(const mu_shader shader);

#endif