#include "stdafx.h"
#include "mu_navigation.h"
#include "mu_terrain.h"

#include "DetourCommon.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

NEXTMU_INLINE mu_boolean inRange(const mu_float *v1, const mu_float *v2, const mu_float r, const mu_float h)
{
	const mu_float dx = v2[0] - v1[0];
	const mu_float dy = v2[1] - v1[1];
	const mu_float dz = v2[2] - v1[2];
	return (dx * dx + dz * dz) < r * r && glm::abs(dy) < h;
}

NEXTMU_INLINE mu_boolean getSteerTarget(
	const dtNavMeshQuery *navQuery,
	const mu_float *startPos, const mu_float *endPos,
	const mu_float minTargetDist,
	const dtPolyRef *path, const mu_int32 pathSize,
	mu_float *steerPos, mu_uint8 &steerPosFlag, dtPolyRef &steerPosRef,
	mu_float *outPoints = 0, mu_int32 *outPointCount = 0
)
{
	// Find steer target.
	static const mu_int32 MAX_STEER_POINTS = 3;
	mu_float steerPath[MAX_STEER_POINTS * 3];
	mu_uint8 steerPathFlags[MAX_STEER_POINTS];
	dtPolyRef steerPathPolys[MAX_STEER_POINTS];
	mu_int32 nsteerPath = 0;
	navQuery->findStraightPath(startPos, endPos, path, pathSize,
							   steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS);
	if (!nsteerPath)
		return false;

	if (outPoints && outPointCount)
	{
		*outPointCount = nsteerPath;
		for (mu_int32 i = 0; i < nsteerPath; ++i)
			dtVcopy(&outPoints[i * 3], &steerPath[i * 3]);
	}

	// Find vertex far enough to steer to.
	mu_int32 ns = 0;
	while (ns < nsteerPath)
	{
		// Stop at Off-Mesh link or when point is further than slop away.
		if (
			(steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0 ||
			!inRange(&steerPath[ns * 3], startPos, minTargetDist, 1000.0f)
			)
			break;
		ns++;
	}
	// Failed to find good point to steer to.
	if (ns >= nsteerPath)
		return false;

	dtVcopy(steerPos, &steerPath[ns * 3]);
	steerPos[1] = startPos[1];
	steerPosFlag = steerPathFlags[ns];
	steerPosRef = steerPathPolys[ns];

	return true;
}

NEXTMU_INLINE mu_int32 fixupCorridor(dtPolyRef *path, const mu_int32 npath, const mu_int32 maxPath,
									const dtPolyRef *visited, const mu_int32 nvisited)
{
	mu_int32 furthestPath = -1;
	mu_int32 furthestVisited = -1;

	// Find furthest common polygon.
	for (mu_int32 i = npath - 1; i >= 0; --i)
	{
		bool found = false;
		for (mu_int32 j = nvisited - 1; j >= 0; --j)
		{
			if (path[i] == visited[j])
			{
				furthestPath = i;
				furthestVisited = j;
				found = true;
			}
		}
		if (found)
			break;
	}

	// If no intersection found just return current path. 
	if (furthestPath == -1 || furthestVisited == -1)
		return npath;

	// Concatenate paths.	

	// Adjust beginning of the buffer to include the visited.
	const mu_int32 req = nvisited - furthestVisited;
	const mu_int32 orig = dtMin(furthestPath + 1, npath);
	mu_int32 size = dtMax(0, npath - orig);
	if (req + size > maxPath)
		size = maxPath - req;
	if (size)
		mu_memmove(path + req, path + orig, size * sizeof(dtPolyRef));

	// Store visited
	for (mu_int32 i = 0; i < req; ++i)
		path[i] = visited[(nvisited - 1) - i];

	return req + size;
}

NEXTMU_INLINE mu_int32 fixupShortcuts(dtPolyRef *path, mu_int32 npath, const dtNavMeshQuery *navQuery)
{
	if (npath < 3)
		return npath;

	// Get connected polygons
	constexpr mu_int32 maxNeis = 16;
	dtPolyRef neis[maxNeis];
	mu_int32 nneis = 0;

	const dtMeshTile *tile = 0;
	const dtPoly *poly = 0;
	if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
		return npath;

	for (mu_uint32 k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
	{
		const dtLink *link = &tile->links[k];
		if (link->ref != 0)
		{
			if (nneis < maxNeis)
				neis[nneis++] = link->ref;
		}
	}

	// If any of the neighbour polygons is within the next few polygons
	// in the path, short cut to that polygon directly.
	constexpr mu_int32 maxLookAhead = 6;
	mu_int32 cut = 0;
	for (mu_int32 i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
		for (mu_int32 j = 0; j < nneis; j++)
		{
			if (path[i] == neis[j]) {
				cut = i;
				break;
			}
		}
	}
	if (cut > 1)
	{
		mu_int32 offset = cut - 1;
		npath -= offset;
		for (mu_int32 i = 1; i < npath; i++)
			path[i] = path[i + offset];
	}

	return npath;
}

namespace MUNavigation
{
	const mu_float NavMeshHalfExtents[3] = { 2.0f, 4.0f, 2.0f };
	dtQueryFilter NavMeshFilter;

	const mu_boolean Initialize()
	{
		NavMeshFilter.setIncludeFlags(0xFFFF);
		NavMeshFilter.setExcludeFlags(0);
		NavMeshFilter.setAreaCost(0, 1.0f);
		return true;
	}

	void Destroy()
	{

	}

	const mu_boolean CreateNavMesh(const mu_utf8string filename, dtNavMesh **navMesh)
	{
		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
		{
			mu_error("navigation file not found ({})", filename);
			return false;
		}

		mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
		std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
		SDL_RWread(fp, buffer.get(), fileLength, 1);
		SDL_RWclose(fp);

		dtNavMesh *tmpNavMesh = dtAllocNavMesh();
		if (tmpNavMesh == nullptr)
		{
			mu_error("failed to allocate nav mesh ({})", filename);
			return false;
		}

		dtStatus status = tmpNavMesh->init(buffer.get(), fileLength, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFreeNavMesh(tmpNavMesh);
			return false;
		}

		*navMesh = tmpNavMesh;

		return true;
	}

	void ReleaseNavMesh(dtNavMesh **navMesh)
	{
		if (navMesh == nullptr || *navMesh == nullptr) return;
		dtFreeNavMesh(*navMesh);
		*navMesh = nullptr;
	}

	const mu_boolean CreateNavMeshQuery(const dtNavMesh *navMesh, const mu_uint32 navMeshQueryCount, dtNavMeshQuery **navMeshQuery)
	{
		for (mu_uint32 n = 0; n < navMeshQueryCount; ++n)
		{
			navMeshQuery[n] = dtAllocNavMeshQuery();
			if (!navMeshQuery[n])
			{
				ReleaseNavMeshQuery(n, navMeshQuery);
				return false;
			}

			dtStatus status = navMeshQuery[n]->init(navMesh, 2048);
			if (dtStatusFailed(status))
			{
				ReleaseNavMeshQuery(n + 1, navMeshQuery);
				return false;
			}
		}

		return true;
	}

	void ReleaseNavMeshQuery(const mu_uint32 navMeshQueryCount, dtNavMeshQuery **navMeshQuery)
	{
		if (navMeshQueryCount == 0u) return;
		for (mu_uint32 n = 0; n < navMeshQueryCount; ++n)
		{
			if (navMeshQuery[n] == nullptr) continue;
			dtFreeNavMeshQuery(navMeshQuery[n]);
			navMeshQuery[n] = nullptr;
		}
	}

	const mu_boolean FindShortestPath(
		const dtNavMesh *navMesh,
		const dtNavMeshQuery *navMeshQuery,
		glm::vec2 startPosition,
		glm::vec2 endPosition,
		NNavPolys *navPolys,
		NNavPath *navPath,
		mu_float distance
	)
	{
		dtPolyRef startPolygon = 0;
		dtPolyRef endPolygon = 0;

		mu_float start[3] = { startPosition[0] * TerrainScaleInv, 0.0f, -startPosition[1] * TerrainScaleInv };
		mu_float end[3] = { endPosition[0] * TerrainScaleInv, 0.0f, -endPosition[1] * TerrainScaleInv };

		// Start Position Polygon
		{
			dtStatus status = navMeshQuery->findNearestPoly(start, NavMeshHalfExtents, &NavMeshFilter, &startPolygon, 0);
			if (dtStatusFailed(status))
			{
				return false;
			}
		}

		// End Position Polygon
		{
			dtStatus status = navMeshQuery->findNearestPoly(end, NavMeshHalfExtents, &NavMeshFilter, &endPolygon, 0);
			if (dtStatusFailed(status))
			{
				return false;
			}
		}

		dtStatus status = navMeshQuery->findPath(startPolygon, endPolygon, start, end, &NavMeshFilter, navPolys->Polys, &navPolys->PolysCount, MaxPathPolys);
		if (dtStatusFailed(status))
		{
			return false;
		}

		navPath->PointsCount = 0;
		navPath->CurrentPoint = 1;

		if (navPolys->PolysCount > 0)
		{
			// Iterate over the path to find smooth path on the detail mesh surface.
			dtPolyRef *polys = navPolys->Polys;
			mu_int32 npolys = navPolys->PolysCount;

			mu_float iterPos[3], targetPos[3];
			navMeshQuery->closestPointOnPoly(startPolygon, start, iterPos, 0);
			//navMeshQuery->closestPointOnPoly(polys[npolys - 1], end, targetPos, 0);
			targetPos[0] = end[0], targetPos[1] = end[1], targetPos[2] = end[2];

			constexpr mu_float STEP_SIZE = 0.5f;
			constexpr mu_float SLOP = 0.01f;

			navPath->Points[navPath->PointsCount] = glm::vec2(iterPos[0] * TerrainScale, -iterPos[2] * TerrainScale);
			navPath->PointsCount++;

			// Move towards target a small advancement at a time until target reached or
			// when ran out of memory to store the path.
			while (npolys && navPath->PointsCount < MaxPathPoints)
			{
				// Find location to steer towards.
				mu_float steerPos[3];
				mu_uint8 steerPosFlag;
				dtPolyRef steerPosRef;

				if (
					!getSteerTarget(
						navMeshQuery, iterPos, targetPos, SLOP,
						polys, npolys, steerPos, steerPosFlag, steerPosRef
					)
					)
					break;

				mu_boolean endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
				mu_boolean offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

				// Find movement delta.
				mu_float delta[3], len;
				dtVsub(delta, steerPos, iterPos);
				len = dtMathSqrtf(dtVdot(delta, delta));
				// If the steer target is end of path or off-mesh link, do not move past the location.
				if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
					len = 1;
				else
					len = STEP_SIZE / len;
				mu_float moveTgt[3];
				dtVmad(moveTgt, iterPos, delta, len);

				// Move
				mu_float result[3];
				dtPolyRef visited[16];
				mu_int32 nvisited = 0;
				navMeshQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &NavMeshFilter,
											   result, visited, &nvisited, 16);

				npolys = fixupCorridor(polys, npolys, MaxPathPolys, visited, nvisited);
				npolys = fixupShortcuts(polys, npolys, navMeshQuery);

				mu_float h = 0;
				navMeshQuery->getPolyHeight(polys[0], result, &h);
				result[1] = h;
				dtVcopy(iterPos, result);

				// Handle end of path and off-mesh links when close enough.
				if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
				{
					// Reached end of path.
					dtVcopy(iterPos, targetPos);
					if (navPath->PointsCount < MaxPathPoints)
					{
						navPath->Points[navPath->PointsCount] = glm::vec2(iterPos[0] * TerrainScale, -iterPos[2] * TerrainScale);
						navPath->PointsCount++;
					}
					break;
				}
				else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
				{
					// Reached off-mesh connection.
					mu_float startPos[3], endPos[3];

					// Advance the path up to and over the off-mesh connection.
					dtPolyRef prevRef = 0, polyRef = polys[0];
					mu_int32 npos = 0;
					while (npos < npolys && polyRef != steerPosRef)
					{
						prevRef = polyRef;
						polyRef = polys[npos];
						npos++;
					}
					for (mu_int32 i = npos; i < npolys; ++i)
						polys[i - npos] = polys[i];
					npolys -= npos;

					// Handle the connection.
					dtStatus status = navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
					if (dtStatusSucceed(status))
					{
						if (navPath->PointsCount < MaxPathPoints)
						{
							navPath->Points[navPath->PointsCount] = glm::vec2(startPos[0] * TerrainScale, -startPos[2] * TerrainScale);
							navPath->PointsCount++;
							// Hack to make the dotted path not visible during off-mesh connection.
							if (navPath->PointsCount & 1)
							{
								navPath->Points[navPath->PointsCount] = glm::vec2(startPos[0] * TerrainScale, -startPos[2] * TerrainScale);
								navPath->PointsCount++;
							}
						}
						// Move position at the other side of the off-mesh link.
						dtVcopy(iterPos, endPos);
						mu_float eh = 0.0f;
						navMeshQuery->getPolyHeight(polys[0], iterPos, &eh);
						iterPos[1] = eh;
					}
				}

				// Store results.
				if (navPath->PointsCount < MaxPathPoints)
				{
					navPath->Points[navPath->PointsCount] = glm::vec2(iterPos[0] * TerrainScale, -iterPos[2] * TerrainScale);
					navPath->PointsCount++;
				}
			}
		}

		//distance -= 5.0f; // This prevent distance issue when attacking or doing actions
		if (distance > 0.0f)
		{
			glm::vec2 tPosition = endPosition;
			mu_int32 currentPoint = navPath->PointsCount - 2;
			while (distance > 0.0f)
			{
				glm::vec2 pointPosition = navPath->Points[currentPoint];
				const mu_float d = glm::distance(pointPosition, tPosition);
				const mu_float distanceToSub = glm::min(d, distance);
				distance -= distanceToSub;
				navPath->Points[currentPoint + 1] = glm::mix(tPosition, pointPosition, distanceToSub / d);

				if (distance <= 0.0f || currentPoint < 1)
				{
					break;
				}

				tPosition = pointPosition;
				--navPath->PointsCount;
				--currentPoint;
			}

			if (
				navPath->PointsCount > 1 &&
				navPath->Points[navPath->PointsCount - 2] == navPath->Points[navPath->PointsCount - 1]
				)
				--navPath->PointsCount;
		}

		return true;
	}

	const mu_boolean TestPath(
		const dtNavMesh *navMesh,
		const dtNavMeshQuery *navMeshQuery,
		glm::vec2 startPosition,
		glm::vec2 endPosition
	)
	{
		mu_int32 polysCount = 0;
		dtPolyRef polys[MaxPathPolys];
		dtPolyRef startPolygon = 0;
		dtPolyRef endPolygon = 0;

		mu_float start[3] = { startPosition[0] * TerrainScaleInv, 0.0f, -startPosition[1] * TerrainScaleInv };
		mu_float end[3] = { endPosition[0] * TerrainScaleInv, 0.0f, -endPosition[1] * TerrainScaleInv };

		// Start Position Polygon
		{
			dtStatus status = navMeshQuery->findNearestPoly(start, NavMeshHalfExtents, &NavMeshFilter, &startPolygon, 0);
			if (dtStatusFailed(status))
			{
				return false;
			}
		}

		// End Position Polygon
		{
			dtStatus status = navMeshQuery->findNearestPoly(end, NavMeshHalfExtents, &NavMeshFilter, &endPolygon, 0);
			if (dtStatusFailed(status))
			{
				return false;
			}
		}

		dtStatus status = navMeshQuery->findPath(startPolygon, endPolygon, start, end, &NavMeshFilter, polys, &polysCount, MaxPathPolys);
		if (dtStatusFailed(status))
		{
			return false;
		}

		return true;
	}

	const mu_boolean FindClosestPoint(
		const dtNavMeshQuery *navMeshQuery,
		const dtPolyRef targetPoly,
		glm::vec2 sourcePosition,
		glm::vec2 &targetPosition
	)
	{
		mu_float currentPos[3] = { sourcePosition[0] * TerrainScaleInv, 0.0f, -sourcePosition[1] * TerrainScaleInv };
		mu_float point[3] = {};
		dtStatus status = navMeshQuery->closestPointOnPoly(targetPoly, currentPos, point, nullptr);
		if (dtStatusFailed(status))
		{
			return false;
		}

		targetPosition[0] = point[0] * TerrainScale;
		targetPosition[1] = -point[2] * TerrainScale;

		return true;
	}

	const mu_float CalculateDistance(
		const dtNavMesh *navMesh,
		const dtNavMeshQuery *navMeshQuery,
		NNavPolys *navPolys,
		NNavPath *navPath,
		glm::vec2 startPosition,
		glm::vec2 endPosition
	)
	{
		dtPolyRef startPolygon = 0;
		dtPolyRef endPolygon = 0;

		mu_float start[3] = { startPosition[0] * TerrainScaleInv, 0.0f, -startPosition[1] * TerrainScaleInv };
		mu_float end[3] = { endPosition[0] * TerrainScaleInv, 0.0f, -endPosition[1] * TerrainScaleInv };

		// Start Position Polygon
		{
			dtStatus status = navMeshQuery->findNearestPoly(start, NavMeshHalfExtents, &NavMeshFilter, &startPolygon, 0);
			if (dtStatusFailed(status))
			{
				return (std::numeric_limits<mu_float>::max)();
			}
		}

		// End Position Polygon
		{
			dtStatus status = navMeshQuery->findNearestPoly(end, NavMeshHalfExtents, &NavMeshFilter, &endPolygon, 0);
			if (dtStatusFailed(status))
			{
				return (std::numeric_limits<mu_float>::max)();
			}
		}

		dtStatus status = navMeshQuery->findPath(startPolygon, endPolygon, start, end, &NavMeshFilter, navPolys->Polys, &navPolys->PolysCount, MaxPathPolys);
		if (dtStatusFailed(status))
		{
			return (std::numeric_limits<mu_float>::max)();
		}

		navPath->PointsCount = 0;

		if (navPolys->PolysCount > 0)
		{
			// Iterate over the path to find smooth path on the detail mesh surface.
			dtPolyRef *polys = navPolys->Polys;
			mu_int32 npolys = navPolys->PolysCount;

			mu_float iterPos[3], targetPos[3];
			navMeshQuery->closestPointOnPoly(startPolygon, start, iterPos, 0);
			//navMeshQuery->closestPointOnPoly(polys[npolys - 1], end, targetPos, 0);
			targetPos[0] = end[0], targetPos[1] = end[1], targetPos[2] = end[2];

			static const mu_float STEP_SIZE = 0.5f;
			static const mu_float SLOP = 0.01f;

			navPath->Points[navPath->PointsCount] = glm::vec2(iterPos[0] * TerrainScale, -iterPos[2] * TerrainScale);
			navPath->PointsCount++;

			// Move towards target a small advancement at a time until target reached or
			// when ran out of memory to store the path.
			while (npolys && navPath->PointsCount < MaxPathPoints)
			{
				// Find location to steer towards.
				mu_float steerPos[3];
				mu_uint8 steerPosFlag;
				dtPolyRef steerPosRef;

				if (
					!getSteerTarget(
						navMeshQuery, iterPos, targetPos, SLOP,
						polys, npolys, steerPos, steerPosFlag, steerPosRef
					)
					)
					break;

				mu_boolean endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
				mu_boolean offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

				// Find movement delta.
				mu_float delta[3], len;
				dtVsub(delta, steerPos, iterPos);
				len = dtMathSqrtf(dtVdot(delta, delta));
				// If the steer target is end of path or off-mesh link, do not move past the location.
				if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
					len = 1;
				else
					len = STEP_SIZE / len;
				mu_float moveTgt[3];
				dtVmad(moveTgt, iterPos, delta, len);

				// Move
				mu_float result[3];
				dtPolyRef visited[16];
				mu_int32 nvisited = 0;
				navMeshQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &NavMeshFilter,
											   result, visited, &nvisited, 16);

				npolys = fixupCorridor(polys, npolys, MaxPathPolys, visited, nvisited);
				npolys = fixupShortcuts(polys, npolys, navMeshQuery);

				mu_float h = 0;
				navMeshQuery->getPolyHeight(polys[0], result, &h);
				result[1] = h;
				dtVcopy(iterPos, result);

				// Handle end of path and off-mesh links when close enough.
				if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
				{
					// Reached end of path.
					dtVcopy(iterPos, targetPos);
					if (navPath->PointsCount < MaxPathPoints)
					{
						navPath->Points[navPath->PointsCount] = glm::vec2(iterPos[0] * TerrainScale, -iterPos[2] * TerrainScale);
						navPath->PointsCount++;
					}
					break;
				}
				else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
				{
					// Reached off-mesh connection.
					mu_float startPos[3], endPos[3];

					// Advance the path up to and over the off-mesh connection.
					dtPolyRef prevRef = 0, polyRef = polys[0];
					mu_int32 npos = 0;
					while (npos < npolys && polyRef != steerPosRef)
					{
						prevRef = polyRef;
						polyRef = polys[npos];
						npos++;
					}
					for (mu_int32 i = npos; i < npolys; ++i)
						polys[i - npos] = polys[i];
					npolys -= npos;

					// Handle the connection.
					dtStatus status = navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
					if (dtStatusSucceed(status))
					{
						if (navPath->PointsCount < MaxPathPoints)
						{
							navPath->Points[navPath->PointsCount] = glm::vec2(startPos[0] * TerrainScale, -startPos[2] * TerrainScale);
							navPath->PointsCount++;
							// Hack to make the dotted path not visible during off-mesh connection.
							if (navPath->PointsCount & 1)
							{
								navPath->Points[navPath->PointsCount] = glm::vec2(startPos[0] * TerrainScale, -startPos[2] * TerrainScale);
								navPath->PointsCount++;
							}
						}
						// Move position at the other side of the off-mesh link.
						dtVcopy(iterPos, endPos);
						mu_float eh = 0.0f;
						navMeshQuery->getPolyHeight(polys[0], iterPos, &eh);
						iterPos[1] = eh;
					}
				}

				// Store results.
				if (navPath->PointsCount < MaxPathPoints)
				{
					navPath->Points[navPath->PointsCount] = glm::vec2(iterPos[0] * TerrainScale, -iterPos[2] * TerrainScale);
					navPath->PointsCount++;
				}
			}
		}

		glm::vec2 currentPosition = navPath->Points[0];
		mu_int32 currentPoint = 1;
		mu_float distance = 0.0f;
		while (currentPoint < navPath->PointsCount)
		{
			glm::vec2 pointPosition = navPath->Points[currentPoint++];
			distance += glm::distance(currentPosition, pointPosition);
			currentPosition = pointPosition;
		}

		return distance;
	}
};