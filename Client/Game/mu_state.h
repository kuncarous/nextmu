#ifndef __MU_STATE_H__
#define __MU_STATE_H__

#pragma once

namespace MUState
{
	void SetTime(const mu_float worldTime, const mu_float elapsedTime);
	void SetUpdate(const mu_float updateTime, const mu_uint32 updateCount);

	const mu_float GetWorldTime();
	const mu_float GetElapsedTime();
	const mu_float GetUpdateTime();
	const mu_uint32 GetUpdateCount();
};

#endif