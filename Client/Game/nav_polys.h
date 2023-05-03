#ifndef __NAV_POLYS_H__
#define __NAV_POLYS_H__

#pragma once

constexpr mu_uint32 MaxPathPolys = 128u;

struct NNavPolys
{
	mu_int32 PolysCount;
	dtPolyRef Polys[MaxPathPolys];
};

#endif