#ifndef __T_THREADING_HELPER_H__
#define __T_THREADING_HELPER_H__

#pragma once

struct TThreadRange
{
	mu_uint32 start;
	mu_uint32 end;
};

namespace TThreading
{
	void SplitLoopIndex(const mu_uint32 count, std::vector<TThreadRange> &threadsRange);
}

#endif