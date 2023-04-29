#ifndef __T_GRAPHICS_SHADER_H__
#define __T_GRAPHICS_SHADER_H__

#pragma once

struct NPipelineResource;
struct NCombinedShader
{
	Diligent::InputLayoutDesc Layout;
	Diligent::RefCntAutoPtr<Diligent::IShader> Vertex;
	Diligent::RefCntAutoPtr<Diligent::IShader> Pixel;
	const NPipelineResource *Resource = nullptr;
	std::vector<Diligent::IPipelineResourceSignature *> ResourceSignatures;
};

typedef mu_uint16 mu_shader;
constexpr mu_shader NInvalidShader = NInvalidUInt16;

mu_shader RegisterShader(NCombinedShader shader);
NCombinedShader *GetShader(const mu_shader shader);

#endif