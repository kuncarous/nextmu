#include "stdafx.h"
#include "mu_capabilities.h"

namespace MUCapabilities
{
	mu_boolean HomogeneousDepth = false;

	const mu_boolean Configure()
	{
		auto caps = bgfx::getCaps();
		HomogeneousDepth = caps->homogeneousDepth;

		return true;
	}

	const mu_boolean IsHomogeneousDepth()
	{
		return HomogeneousDepth;
	}
}