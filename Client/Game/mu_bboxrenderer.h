#ifndef __MU_BBOXRENDERER_H__
#define __MU_BBOXRENDERER_H__

#pragma once

#include "mu_rendererconfig.h"
#include "mu_math_aabb.h"

namespace MUBBoxRenderer
{
	const mu_boolean Initialize();
	void Destroy();

	void Render(
		NBoundingBox &bbox
	);
}

#endif