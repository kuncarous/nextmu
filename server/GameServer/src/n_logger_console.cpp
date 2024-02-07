#include "mu_precompiled.h"
#include "n_logger_console.h"

void NConsoleLogger::Write(NLogMessagePtr message)
{
    QMutexLocker lock(&Mutex);
    Messages.push_back(std::move(message));
}

std::vector<NLogMessagePtr> NConsoleLogger::GetMessages()
{
    QMutexLocker lock(&Mutex);
    return std::move(Messages);
}
