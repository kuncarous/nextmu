#ifndef __MU_BBOXRENDERER_H__
#define __MU_BBOXRENDERER_H__

#pragma once

#include "mu_math_obb.h"

namespace MUBBoxRenderer
{
	mu_boolean Initialize();
	void Destroy();

	void Reset();
	void Render(const NBoundingBox &aabb);
	void Render(const NOrientedBoundingBox &obb);
}

#endif