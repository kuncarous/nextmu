#include "stdafx.h"
#include "mu_math_frustum.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static glm::vec4 PlanesPoint[NFrustumPlane::Count] = {
	glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
	glm::vec4(-1.0f, 0.0f, 1.0f, 1.0f),
	glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
	glm::vec4(0.0f, -1.0f, 1.0f, 1.0f),
	glm::vec4(0.0f,	0.0f, 0.0f, 1.0f),
	glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
};

static glm::vec4 DefaultOrigin(0.0f, 0.0f, 0.0f, 1.0f);
static glm::quat DefaultOrientation(0.0f, 0.0f, 0.0f, 1.0f);

void NMathFrustum::Update(cglm::mat4 view, cglm::mat4 projection)
{
	cglm::mat4 viewProj, invViewProj;

	cglm::glm_mat4_mul(projection, view, viewProj);
	cglm::glm_mat4_inv(viewProj, invViewProj);

	cglm::glm_frustum_planes(
		viewProj,
		Planes
	);
	cglm::glm_frustum_corners(
		invViewProj,
		Corners
	);

	cglm::mat4 identity;
	cglm::glm_mat4_identity(identity);
	cglm::glm_frustum_box(
		Corners,
		identity,
		Box
	);
}

mu_boolean NMathFrustum::IsBoxVisible(const glm::vec3 &min, const glm::vec3 &max) const
{
	cglm::vec3 box[2] = {
		{
			min[0] < max[0] ? min[0] : max[0],
			min[1] < max[1] ? min[1] : max[1],
			min[2] < max[2] ? min[2] : max[2],
		},
		{
			min[0] > max[0] ? min[0] : max[0],
			min[1] > max[1] ? min[1] : max[1],
			min[2] > max[2] ? min[2] : max[2],
		},
	};
	return cglm::glm_aabb_frustum(box, const_cast<cglm::vec4 *>(Planes));
}