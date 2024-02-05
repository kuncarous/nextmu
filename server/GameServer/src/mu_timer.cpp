#include "mu_precompiled.h"
#include "mu_timer.h"
#include <chrono>
#include <thread>

constexpr mu_int32 MaxServerFPS = 120;
constexpr mu_double MinimumFPSTime = (1000.0 / static_cast<mu_double>(MaxServerFPS));

namespace MUGlobalTimer
{
    std::chrono::high_resolution_clock::time_point BaseTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point LastTime = std::chrono::high_resolution_clock::now();
	mu_double ElapsedTime = 0.0;
    mu_double WorkTime = 0.0;

	void Wait()
	{
        std::chrono::high_resolution_clock::time_point lastTime = LastTime;
        LastTime = std::chrono::high_resolution_clock::now();

		std::chrono::duration<mu_double, std::milli> work_time = LastTime - lastTime;
		WorkTime = work_time.count();

        if (work_time.count() < MinimumFPSTime)
        {
            std::chrono::duration<mu_double, std::milli> delta_ms(MinimumFPSTime - work_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(delta_ms_duration);
            LastTime = std::chrono::high_resolution_clock::now();
        }

		ElapsedTime = std::chrono::duration<mu_double, std::milli>(LastTime - lastTime).count();
	}

	const mu_double GetWorkFrametime()
	{
		return WorkTime;
	}

	const mu_double GetElapsedFrametime()
	{
		return ElapsedTime;
	}

	const mu_double GetFrametime()
	{
		return std::chrono::duration<mu_double, std::milli>(LastTime - BaseTime).count();
	}

	const mu_double GetRealtime()
	{
        const auto currentTime = std::chrono::high_resolution_clock::now();
		const auto timeSinceStart = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - BaseTime);
		return mu_double(timeSinceStart.count() / 1000000.0);
	}
};
