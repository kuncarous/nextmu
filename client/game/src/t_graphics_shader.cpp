#include "mu_precompiled.h"
#include "t_graphics_shader.h"

std::vector<NCombinedShader> Shaders;

mu_shader RegisterShader(NCombinedShader shader)
{
	const mu_shader index = static_cast<mu_shader>(Shaders.size());
	Shaders.push_back(shader);
	return index;
}

NCombinedShader *GetShader(const mu_shader shader)
{
	return &Shaders[shader];
}