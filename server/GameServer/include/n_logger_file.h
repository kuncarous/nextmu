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
    explicit NFileLogger(const QString directory, NConsoleLogger *consoleLogger = nullptr);
    virtual ~NFileLogger();

private:
    void Close();
    void Create(const QDateTime currentDT = QDateTime::currentDateTime());

public:
    void Write(NLogMessagePtr message);

public:
	template<typename... Args>
    void Write(const mu_utf8string type, fmt::format_string<Args...> format, Args&&... args)
    {
        Write(std::make_unique<NLogMessage>(type, fmt::format(format, std::forward<Args>(args)...).c_str()));
	}

    void Write(const mu_utf8string type, const mu_char *message)
    {
        Write(std::make_unique<NLogMessage>(type, message));
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
