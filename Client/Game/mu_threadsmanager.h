#ifndef __MU_THREADSMANAGER_H__
#define __MU_THREADSMANAGER_H__

#pragma once

namespace MUThreadsManager
{
	typedef std::function<void(const mu_uint32 threadIndex)> RunFunction;

	const mu_boolean Initialize();
	void Destroy();

	const mu_uint32 GetThreadsCount();
	void Run(RunFunction func);
}

#endif