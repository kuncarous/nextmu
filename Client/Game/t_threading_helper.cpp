#include "stdafx.h"
#include "t_threading_helper.h"

namespace TThreading
{
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