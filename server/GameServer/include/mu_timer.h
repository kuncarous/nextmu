#ifndef __MU_TIMER_H__
#define __MU_TIMER_H__

#pragma once

namespace MUGlobalTimer
{
	void Wait();

	const mu_double GetWorkFrametime();
	const mu_double GetElapsedFrametime();
	const mu_double GetFrametime();
	const mu_double GetRealtime();
};

#endif