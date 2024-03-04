#ifndef __MU_MATH_OBB_H__
#define __MU_MATH_OBB_H__

class NBoundingBox;
struct NCompressedMatrix;

namespace OBBVertexIndex
{
	typedef mu_uint32 Type;
	enum : Type
	{
	eBX_BY_BZ,
	eBX_BY_TZ,
	eBX_TY_BZ,
	eBX_TY_TZ,
	eTX_BY_BZ,
	eTX_BY_TZ,
	eTX_TY_BZ,
	eTX_TY_TZ,
	};
};

class NOrientedBoundingBox
{
public:
	constexpr static mu_uint32 VerticesCount = 8;

public:
	glm::vec3 Vertices[VerticesCount];

public:
	NOrientedBoundingBox();
	NOrientedBoundingBox(const glm::vec3 vertices[VerticesCount]);
	NOrientedBoundingBox(const NOrientedBoundingBox &b);	
	NOrientedBoundingBox(const NBoundingBox &b);

	[[nodiscard]] NOrientedBoundingBox Transform(const NCompressedMatrix &matrix) const;
};

#endif
