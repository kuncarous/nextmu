#include "mu_precompiled.h"
#include "mu_timer.h"
#include "mu_config.h"

#include <array>
#include <bitset>
#include <thread>

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
#include <unistd.h>
#endif

#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
constexpr mu_int32 MaxGameFPS = 30; // 30 FPS (Mobile devices only)
constexpr mu_int32 MaxBackgroundFPS = 30;
#else
constexpr mu_int32 MaxGameFPS = 60;
constexpr mu_int32 MaxBackgroundFPS = 30;
#endif
constexpr mu_double MinimumFPSTime = (1000.0 / static_cast<mu_double>(MaxGameFPS));
constexpr mu_double MinimumBackgroundFPSTime = (1000.0 / static_cast<mu_double>(MaxBackgroundFPS));

namespace MUGlobalTimer
{
	std::chrono::steady_clock::time_point BaseTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point LastTime = std::chrono::steady_clock::now();
	mu_double ElapsedTime = 0.0;
	mu_double WorkTime = 0.0;
	mu_boolean LimitFPS = false;

	void Wait()
	{
		std::chrono::steady_clock::time_point lastTime = LastTime;
		LastTime = std::chrono::steady_clock::now();

		std::chrono::duration<mu_double, std::milli> work_time = LastTime - lastTime;
		WorkTime = work_time.count();

		const mu_boolean active = true;
#if NEXTMU_OPERATING_SYSTEM_TYPE == NEXTMU_OSTYPE_MOBILE
		if (
			LimitFPS == true ||
			active == false
		)
#else
		if (
			(
				MUConfig::GetVerticalSync() == false &&
				LimitFPS == true
			) ||
			active == false
		)
#endif
		{
			const mu_double fpsTime = active == false ? MinimumBackgroundFPSTime : MinimumFPSTime;
			if (work_time.count() < fpsTime)
			{
				std::chrono::duration<mu_double, std::milli> delta_ms(fpsTime - work_time.count());
				auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
				usleep(static_cast<mu_uint32>(delta_ms_duration.count()));
#else
				std::this_thread::sleep_for(delta_ms_duration);
#endif
				LastTime = std::chrono::steady_clock::now();
			}
		}

		ElapsedTime = std::chrono::duration<mu_double, std::milli>(LastTime - lastTime).count();
	}

	void ToggleLimitFPS()
	{
		LimitFPS = !LimitFPS;
	}

	const mu_boolean IsFpsLimited()
	{
		return LimitFPS;
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
		const auto currentTime = std::chrono::steady_clock::now();
		const auto timeSinceStart = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - BaseTime);
		return mu_double(timeSinceStart.count() / 1000000.0);
	}
};
