#include "stdafx.h"
#include "mu_threadsmanager.h"
#include <thread>
#include <barrier>
#include <xutility>

namespace MUThreadsManager
{
	std::atomic_bool Terminated = false;
	std::vector<std::jthread> Threads;
	std::unique_ptr<std::barrier<>> WakeBarrier;
	std::unique_ptr<std::barrier<>> RunBarrier;
	RunFunction Function;

	void Worker(const mu_uint32 index);

	const mu_boolean Initialize()
	{
		const auto threadsCount = glm::min(std::jthread::hardware_concurrency(), 8u);

		WakeBarrier.reset(new std::barrier(threadsCount + 1));
		RunBarrier.reset(new std::barrier(threadsCount + 1));

		Threads.resize(threadsCount);
		for (mu_uint32 n = 0; n < threadsCount; ++n)
		{
			Threads[n] = std::jthread(Worker, n);
		}

		return true;
	}

	void Destroy()
	{
		Terminated = true;
		WakeBarrier->arrive_and_wait();
		Threads.clear();
		WakeBarrier.reset();
		RunBarrier.reset();
	}

	const mu_uint32 GetThreadsCount()
	{
		return static_cast<mu_uint32>(Threads.size());
	}

	void Run(RunFunction func)
	{
		Function = std::move(func);
		WakeBarrier->arrive_and_wait();
		RunBarrier->arrive_and_wait();
	}

	void Worker(const mu_uint32 index)
	{
		while (true)
		{
			WakeBarrier->arrive_and_wait();
			if (Terminated) break;
			Function(index);
			RunBarrier->arrive_and_wait();
		}
	}
}