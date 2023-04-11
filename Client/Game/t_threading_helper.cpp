#include "stdafx.h"
#include "t_threading_helper.h"

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

	void SplitLoopIndex(const mu_uint32 elementsCount, std::vector<TThreadRange> &threadsRange)
	{
		const mu_uint32 threadsCount = static_cast<mu_uint32>(threadsRange.size());
		for (mu_uint32 n = 0; n < threadsCount; ++n)
		{
			auto &thread = threadsRange[n];
			GetIndexTasking(n, elementsCount, thread.start, thread.end, threadsCount);
		}
	}
}