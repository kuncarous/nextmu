#include "mu_precompiled.h"
#include "mu_capabilities.h"
#include "mu_graphics.h"

namespace MUCapabilities
{
	mu_boolean HomogeneousDepth = false;

	const mu_boolean Configure()
	{
		const auto device = MUGraphics::GetDevice();
		const auto &deviceInfo = device->GetDeviceInfo();

		HomogeneousDepth = deviceInfo.IsGLDevice();

		return true;
	}

	const mu_boolean IsHomogeneousDepth()
	{
		return HomogeneousDepth;
	}
}