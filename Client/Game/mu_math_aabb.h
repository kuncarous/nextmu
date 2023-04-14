#ifndef __MU_MATH_AABB_H__
#define __MU_MATH_AABB_H__

#pragma once

constexpr mu_float InvalidBox = 50000.0f;

template<typename T>
NEXTMU_INLINE void Swap(T &a, T &b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

class NBoundingBox
{
public:
	glm::vec3 Min;
	glm::vec3 Max;

public:
	void Order()
	{
		if (Min.x > Max.x) Swap(Min.x, Max.x);
		if (Min.y > Max.y) Swap(Min.y, Max.y);
		if (Min.z > Max.z) Swap(Min.z, Max.z);
	}
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