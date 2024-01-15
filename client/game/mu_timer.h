#ifndef __MU_TIMER_H__
#define __MU_TIMER_H__

#pragma once

namespace MUGlobalTimer
{
	void Wait();

	void ToggleLimitFPS();
	const mu_boolean IsFpsLimited();

	const mu_double GetWorkFrametime();
	const mu_double GetElapsedFrametime();
	const mu_double GetFrametime();
	const mu_double GetRealtime();
};

#endif