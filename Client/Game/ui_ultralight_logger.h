#ifndef __UI_ULTRALIGHT_LOGGER_H__
#define __UI_ULTRALIGHT_LOGGER_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
#include <Ultralight/Ultralight.h>

namespace UIUltralight
{
	class ConsoleLogger : public ultralight::Logger
	{
	public:
		virtual void LogMessage(ultralight::LogLevel log_level, const ultralight::String16 &message) override;
	};
};
#endif

#endif