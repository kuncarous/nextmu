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

    void Write(NLogMessagePtr message);
    std::vector<NLogMessagePtr> GetMessages();

private:
    QMutex Mutex;
    std::vector<NLogMessagePtr> Messages;
};

#endif
