#include "mu_precompiled.h"
#include "n_logger_file.h"
#include <QDir>

NFileLogger::NFileLogger(const QString directory, NConsoleLogger *consoleLogger) : HasLogs(false), ConsoleLogger(consoleLogger)
{
    QFileInfo fileInfo(directory + QDir::separator() + "dummy.log");
    fileInfo.absoluteDir().mkpath(".");
    Directory = (fileInfo.absolutePath() + QDir::separator()).toStdString();
}

NFileLogger::~NFileLogger()
{
    Close();
}

void NFileLogger::Close()
{
    if (Stream != nullptr) *Stream << "]";
    Stream.reset();
    File.reset();
}

void NFileLogger::Create(const QDateTime currentDT)
{
    const QDate &currentDate = currentDT.date();

    if (
        GeneratedDate.day() == currentDate.day() &&
        GeneratedDate.month() == currentDate.month() &&
        GeneratedDate.year() == currentDate.year()
    )
    {
        return;
    }

    mu_uint32 logNumber = 1u;
    mu_utf8string filename;

    do
    {
        filename = fmt::format("{}{:04}-{:02}-{:02}-{:04}.json", Directory, currentDate.year(), currentDate.month(), currentDate.day(), logNumber++);
        QFileInfo fileInfo(filename.c_str());
        if (fileInfo.exists() == false) break;
    } while(true);

    if (mu_rwfromfile_swt(File, filename, QIODeviceBase::WriteOnly) == false)
    {
        return;
    }

    GeneratedDate = currentDate;
    HasLogs = false;
    Stream.reset(new_nothrow QTextStream(File.get()));
    *Stream << "[";
}

void NFileLogger::Write(NLogMessagePtr message)
{
    // File Mutex
    {
        QMutexLocker lock(&Mutex);

        const QDateTime &currentDT = message->DateTime;
        Create(currentDT);
        if (Stream == nullptr) return;

        if (HasLogs) *Stream << ",";
        HasLogs = true;

        nlohmann::json jobject;
        jobject["timestamp"] = currentDT.currentMSecsSinceEpoch();
        jobject["type"] = message->Type;
        jobject["message"] = message->Message;
        *Stream << jobject.dump().c_str();
    }

    if (ConsoleLogger != nullptr) ConsoleLogger->Write(std::move(message));
}
