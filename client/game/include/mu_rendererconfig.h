#ifndef __MU_RENDERERCONFIG_H__
#define __MU_RENDERERCONFIG_H__

#pragma once

struct NRenderConfig
{
	mu_uint32 BoneOffset;
	glm::vec3 BodyOrigin;
	mu_float BodyScale;
	mu_boolean EnableLight;
	glm::vec4 BodyLight;
	mu_float BlendMeshLight;
};

#endif