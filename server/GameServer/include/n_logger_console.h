#ifndef __N_LOGGER_CONSOLE_H__
#define __N_LOGGER_CONSOLE_H__

#pragma once

#include "n_logger_message.h"
#include <QMutex>

class NConsoleLogger
{
public:
    NConsoleLogger() {}
    ~NConsoleLogger() {}

    void Write(const NLogMessage &message);
    std::vector<NLogMessage> GetMessages();

private:
    QMutex Mutex;
    std::vector<NLogMessage> Messages;
};

#endif
