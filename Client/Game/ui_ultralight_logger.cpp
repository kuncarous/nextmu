#include "stdafx.h"
#include "ui_ultralight_logger.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
namespace UIUltralight
{
	void ConsoleLogger::LogMessage(ultralight::LogLevel log_level, const ultralight::String16 &message)
	{
		switch (log_level)
		{
		case ultralight::kLogLevel_Error:
			mu_error("[Ultralight] {}", ConvertToUTF8String(message.data()));
			break;

		case ultralight::kLogLevel_Warning:
		case ultralight::kLogLevel_Info:
			mu_info("[Ultralight] {}", ConvertToUTF8String(message.data()));
			break;
		}
	}
};
#endif