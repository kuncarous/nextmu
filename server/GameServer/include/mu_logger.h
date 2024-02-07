#ifndef __MU_LOGGER_H__
#define __MU_LOGGER_H__

#pragma once

#include "n_logger_file.h"

struct NLogTypeConfig
{
    QString Type;
    QString BackgroundColor;
    QString FontColor;
    QString SelectedColor;
    QString HighlightColor;
};

namespace MULogger
{
    const mu_boolean Load();
    const mu_boolean Initialize();
    void Destroy();
    NConsoleLogger *GetConsoleLogger();
    void Write(const mu_utf8string type, const mu_utf8string message);
    const NLogTypeConfig *GetType(const mu_utf8string type);
};

#endif
