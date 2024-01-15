#ifndef __MU_NAVIGATION_H__
#define __MU_NAVIGATION_H__

#pragma once

#include "DetourCommon.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "nav_polys.h"
#include "nav_path.h"

class dtNavMesh;
class dtNavMeshQuery;

namespace MUNavigation
{
	const mu_boolean Initialize();
	void Destroy();

	const mu_boolean CreateNavMesh(const mu_utf8string filename, dtNavMesh **navMesh);
	const mu_boolean CreateNavMeshQuery(const dtNavMesh *navMesh, const mu_uint32 navMeshQueryCount, dtNavMeshQuery **navMeshQuery);
	void ReleaseNavMeshQuery(const mu_uint32 navMeshQueryCount, dtNavMeshQuery **navMeshQuery);

	const mu_boolean FindShortestPath(
		const dtNavMesh *navMesh,
		const dtNavMeshQuery *navMeshQuery,
		glm::vec2 startPosition,
		glm::vec2 endPosition,
		NNavPolys *navPolys,
		NNavPath *navPath,
		mu_float distanceOffset
	);
	const mu_boolean TestPath(
		const dtNavMesh *navMesh,
		const dtNavMeshQuery *navMeshQuery,
		glm::vec2 startPosition,
		glm::vec2 endPosition
	);
	const mu_boolean FindClosestPoint(
		const dtNavMeshQuery *navMeshQuery,
		const dtPolyRef targetPoly,
		glm::vec2 sourcePosition,
		glm::vec2 &targetPosition
	);
	const mu_float CalculateDistance(
		const dtNavMesh *navMesh,
		const dtNavMeshQuery *navMeshQuery,
		NNavPolys *navPolys,
		NNavPath *navPath,
		glm::vec2 startPosition,
		glm::vec2 endPosition
	);
}

#endif