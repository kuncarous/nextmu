#include "stdafx.h"
#include "mu_physics.h"

#if PHYSICS_ENABLED == 1
namespace MUPhysics
{
	physx::PxDefaultErrorCallback gDefaultErrorCallback;
	physx::PxDefaultAllocator gDefaultAllocatorCallback;
	physx::PxFoundation *Foundation = nullptr;
	physx::PxPvd *Pvd = nullptr;
	physx::PxPvdTransport *PvdTransport = nullptr;
	physx::PxPhysics *Physics = nullptr;
	physx::PxCooking *Cooking = nullptr;

	const mu_boolean Initialize()
	{
		Foundation = PxCreateFoundation(
			PX_PHYSICS_VERSION,
			gDefaultAllocatorCallback,
			gDefaultErrorCallback
		);
		if (!Foundation)
		{
			return false;
		}

		Pvd = PxCreatePvd(*Foundation);
		if (!Pvd)
		{
			return false;
		}

		// Debug Only
		/*PvdTransport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
		if (!PvdTransport)
		{
			return false;
		}

		Pvd->connect(*PvdTransport, physx::PxPvdInstrumentationFlag::eALL);*/

		constexpr mu_boolean recordMemoryAllocations = false;
		const auto toleranceScale = physx::PxTolerancesScale();

		Physics = PxCreatePhysics(
			PX_PHYSICS_VERSION,
			*Foundation,
			toleranceScale,
			recordMemoryAllocations,
			Pvd
		);
		if (!Physics)
		{
			return false;
		}

		Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *Foundation, physx::PxCookingParams(toleranceScale));
		if (!Cooking)
		{
			return false;
		}

		if (!PxInitExtensions(*Physics, Pvd))
		{
			return false;
		}

		return true;
	}

	void Destroy()
	{
		PxCloseExtensions();
		if (Cooking) Cooking->release();
		if (Physics) Physics->release();
		if (PvdTransport) PvdTransport->release();
		if (Pvd) Pvd->release();
		if (Foundation) Foundation->release();
	}
};
#endif