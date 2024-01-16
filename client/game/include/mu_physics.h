#ifndef __MU_PHYSICS_H__
#define __MU_PHYSICS_H__

#pragma once

#if PHYSICS_ENABLED == 1
namespace MUPhysics
{
	const mu_boolean Initialize();
	void Destroy();
};
#endif

#endif