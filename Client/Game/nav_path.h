#ifndef __NAV_PATH_H__
#define __NAV_PATH_H__

#pragma once

constexpr mu_uint32 MaxPathPoints = 128u;

class NNavPath
{
public:
	NNavPath() : CurrentPoint(0), PointsCount(0)
	{
	}

public:
	mu_uint32 CurrentPoint;
	mu_uint32 PointsCount;
	glm::vec2 Points[MaxPathPoints];
};

#endif