#include "stdafx.h"
#include "t_graphics_resources.h"
#include "t_graphics_shaderresources.h"

mu_atomic_uint32_t NGraphicsResource::IdGenerator;

NGraphicsResource::NGraphicsResource(const NGraphicsResourceType type) : Type(type), Id(IdGenerator++)
{

}

NGraphicsResource::~NGraphicsResource()
{
	ReleaseShaderResourcesByResourceId(Id);
}

mu_uint32 GenerateResourceId()
{
	return NGraphicsResource::IdGenerator++;
}