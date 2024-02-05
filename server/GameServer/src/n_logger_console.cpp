#include "mu_precompiled.h"
#include "n_logger_console.h"

void NConsoleLogger::Write(const NLogMessage &message)
{
    QMutexLocker lock(&Mutex);
    Messages.push_back(message);
}

std::vector<NLogMessage> NConsoleLogger::GetMessages()
{
    QMutexLocker lock(&Mutex);
    return std::move(Messages);
}
