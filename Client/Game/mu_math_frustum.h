#ifndef __MU_MATH_FRUSTUM_H__
#define __MU_MATH_FRUSTUM_H__

#pragma once

namespace NFrustumPoint
{
	enum : mu_uint32
	{
		RightTopNear,
		RightBottomNear,
		LeftTopNear,
		LeftBottomNear,
		RightTopFar,
		RightBottomFar,
		LeftTopFar,
		LeftBottomFar,
		Count,
	};
}

namespace NFrustumPlane
{
	typedef mu_uint32 Type;
	enum : Type
	{
		Left,
		Right,
		Bottom,
		Top,
		Near,
		Far,
		Count,
	};
}

class NMathFrustum
{
public:
	void Update(cglm::mat4 view, cglm::mat4 projection);

public:
	mu_boolean IsBoxVisible(const glm::vec3 &min, const glm::vec3 &max) const;

private:
	glm::vec3 Origin;
	glm::quat Orientation;
	cglm::vec4 Planes[NFrustumPlane::Count];
	cglm::vec4 Corners[NFrustumPoint::Count];
	cglm::vec3 Box[2];
};

#endif