#ifndef __N_LOGGER_FILE_H__
#define __N_LOGGER_FILE_H__

#pragma once

#include "n_logger_console.h"

class NFileLogger
{
public:
    static constexpr mu_size MaxBuffer = 64 * 1024; // 64KB
    static constexpr mu_size MaxWriteBuffer = 64 * 1024; // 64KB

public:
    explicit NFileLogger(const mu_utf8string directory, NConsoleLogger *consoleLogger = nullptr);
    virtual ~NFileLogger();

private:
    void Close();
    void Create(const QDateTime currentDT = QDateTime::currentDateTime());
    void WriteBuffer(const NLogMessage &message);

public:
	template<typename... Args>
    void Print(const mu_utf8string type, const mu_char *format, const Args &... args)
	{
        WriteBuffer(NLogMessage(type, fmt::format(format, args...)));
	}

private:
    mu_boolean HasLogs;
    QDate GeneratedDate;
    mu_utf8string Directory;
    NConsoleLogger *ConsoleLogger;
    QMutex Mutex;
    std::unique_ptr<QFile> File;
    std::unique_ptr<QTextStream> Stream;
};

#endif
