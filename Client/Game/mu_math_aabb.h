#ifndef __MU_MATH_AABB_H__
#define __MU_MATH_AABB_H__

#pragma once

constexpr mu_float InvalidBox = 50000.0f;

class NBoundingBox
{
public:
	glm::vec3 Min;
	glm::vec3 Max;
};

class NBoundingBoxWithDefault : public NBoundingBox
{
public:
	NBoundingBoxWithDefault()
	{
		Min = glm::vec3(InvalidBox, InvalidBox, InvalidBox);
		Max = glm::vec3(-InvalidBox, -InvalidBox, -InvalidBox);
	}
};

#endif