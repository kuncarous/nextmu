#include "mu_precompiled.h"
#include "mu_math_aabb.h"
#include "mu_math_obb.h"

NBoundingBox::NBoundingBox(const NOrientedBoundingBox &obb)
{
	Min = glm::vec3(InvalidBox, InvalidBox, InvalidBox);
	Max = glm::vec3(-InvalidBox, -InvalidBox, -InvalidBox);

	for (mu_uint32 n = 0; n < NOrientedBoundingBox::VerticesCount; ++n)
	{
		const auto &v = obb.Vertices[n];

		if (v.x < Min.x) Min.x = v.x;
		if (v.y < Min.y) Min.y = v.y;
		if (v.z < Min.z) Min.z = v.z;

		if (v.x > Max.x) Max.x = v.x;
		if (v.y > Max.y) Max.y = v.y;
		if (v.z > Max.z) Max.z = v.z;
	}
}
