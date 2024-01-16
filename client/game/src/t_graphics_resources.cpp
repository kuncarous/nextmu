#include "mu_precompiled.h"
#include "t_graphics_resources.h"
#include "t_graphics_shaderresources.h"

NGraphicsResource::NGraphicsResource(const NGraphicsResourceType type) : Type(type), Id(GenerateResourceId())
{

}

NGraphicsResource::~NGraphicsResource()
{
	ShaderResourcesBindingManager.ReleaseShaderResourcesByResourceId(Id);
}