#include "stdafx.h"
#include "t_graphics_resourceid.h"

mu_atomic_uint32_t IdGenerator(0u);
mu_uint32 GenerateResourceId()
{
	return IdGenerator++;
}