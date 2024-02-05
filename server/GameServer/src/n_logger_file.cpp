#include "mu_precompiled.h"
#include "n_logger_file.h"

NFileLogger::NFileLogger(const mu_utf8string directory, NConsoleLogger *consoleLogger) : HasLogs(false), Directory(directory), ConsoleLogger(consoleLogger)
{
    
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

    if (mu_rwfromfile_swt(File, fmt::format("{}/{:04}-{:02}-{:02}.json", Directory, currentDate.year(), currentDate.month(), currentDate.day()), QIODeviceBase::WriteOnly) == false)
    {
        return;
    }

    GeneratedDate = currentDate;
    HasLogs = false;
    Stream.reset(new_nothrow QTextStream(File.get()));
    *Stream << "[";
}

void NFileLogger::WriteBuffer(const NLogMessage &message)
{
    QMutexLocker lock(&Mutex);

    const QDateTime &currentDT = message.DateTime;
    Create(currentDT);
    if (Stream == nullptr) return;

    if (HasLogs) *Stream << ",";
    HasLogs = true;

    nlohmann::json jobject;
    jobject["timestamp"] = currentDT.currentMSecsSinceEpoch();
    jobject["type"] = message.Type;
    jobject["message"] = message.Message;
    *Stream << jobject.dump().c_str();

    if (ConsoleLogger != nullptr) ConsoleLogger->Write(message);
}
