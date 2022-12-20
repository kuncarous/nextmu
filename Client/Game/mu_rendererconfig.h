#ifndef __MU_RENDERERCONFIG_H__
#define __MU_RENDERERCONFIG_H__

#pragma once

struct NRenderConfig
{
	const mu_uint32 BoneOffset;
	const glm::vec3 BodyOrigin;
	const mu_float BodyScale;
	const mu_boolean EnableLight;
	const glm::vec4 BodyLight;
};

#endif