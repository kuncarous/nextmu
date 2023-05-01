#ifndef __NAV_PATH_H__
#define __NAV_PATH_H__

#pragma once

constexpr mu_uint32 MaxPathPoints = 2048u;

class NNavPath
{
public:
	NNavPath() : CurrentPoint(0), PointsCount(0)
	{
		Points.reserve(20);
	}

public:
	mu_uint32 CurrentPoint;
	mu_uint32 PointsCount;
	std::vector<glm::vec2> Points;
};

#endif