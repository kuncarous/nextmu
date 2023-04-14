#ifndef __T_THREADING_HELPER_H__
#define __T_THREADING_HELPER_H__

#pragma once

namespace TThreading
{
	NEXTMU_INLINE void GetIndexTasking(mu_uint32 thread, mu_uint32 elementsCount, mu_uint32 &start, mu_uint32 &end, mu_uint32 threadsCount)
	{
		const mu_uint32 remainders = elementsCount % threadsCount;
		const mu_uint32 itemsPerThread = elementsCount / threadsCount;
		const mu_uint32 extraThisThread = thread < remainders;
		const mu_uint32 adder = extraThisThread ? thread : remainders;

		start = itemsPerThread * thread + adder;
		end = start + itemsPerThread + extraThisThread;
	}
}

#endif