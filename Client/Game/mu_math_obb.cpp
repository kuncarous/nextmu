#include "stdafx.h"
#include "mu_math_obb.h"

NOrientedBoundingBox::NOrientedBoundingBox()
{
	mu_zeromem(Vertices, sizeof(Vertices));
}

NOrientedBoundingBox::NOrientedBoundingBox(const glm::vec3 vertices[VerticesCount])
{
	for (mu_uint32 n = 0; n < VerticesCount; ++n)
	{
		Vertices[n] = vertices[n];
	}
}

NOrientedBoundingBox::NOrientedBoundingBox(const NOrientedBoundingBox &b)
{
	mu_memcpy(Vertices, b.Vertices, sizeof(Vertices));
}

NOrientedBoundingBox::NOrientedBoundingBox(const NBoundingBox &b)
{
	const glm::vec3 center = (b.Min + b.Max) * 0.5f;
	const glm::vec3 diffN = b.Min - center;
	const glm::vec3 diffP = b.Max - center;
	
	Vertices[OBBVertexIndex::eBX_BY_BZ] = center + glm::vec3(diffN.x, diffN.y, diffN. z);
	Vertices[OBBVertexIndex::eBX_BY_TZ] = center + glm::vec3(diffN.x, diffN.y, diffP. z);
	Vertices[OBBVertexIndex::eBX_TY_BZ] = center + glm::vec3(diffN.x, diffP.y, diffN. z);
	Vertices[OBBVertexIndex::eBX_TY_TZ] = center + glm::vec3(diffN.x, diffP.y, diffP. z);
	Vertices[OBBVertexIndex::eTX_BY_BZ] = center + glm::vec3(diffP.x, diffN.y, diffN. z);
	Vertices[OBBVertexIndex::eTX_BY_TZ] = center + glm::vec3(diffP.x, diffN.y, diffP. z);
	Vertices[OBBVertexIndex::eTX_TY_BZ] = center + glm::vec3(diffP.x, diffP.y, diffN. z);
	Vertices[OBBVertexIndex::eTX_TY_TZ] = center + glm::vec3(diffP.x, diffP.y, diffP. z);
}

[[nodiscard]] NOrientedBoundingBox NOrientedBoundingBox::Transform(const NCompressedMatrix &matrix) const
{
	NOrientedBoundingBox output;
	for (mu_uint32 n = 0; n < NOrientedBoundingBox::VerticesCount; ++n)
	{
		output.Vertices[n] = ::Transform(Vertices[n], matrix);
	}
	return output;
}