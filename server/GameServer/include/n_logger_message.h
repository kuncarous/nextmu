#ifndef __N_LOGGER_MESSAGE_H__
#define __N_LOGGER_MESSAGE_H__

#pragma once

namespace NLogType
{
    extern const mu_utf8string Message;
    extern const mu_utf8string Success;
    extern const mu_utf8string Warning;
    extern const mu_utf8string Error;
};

class NLogMessage
{
public:
    explicit NLogMessage(const mu_utf8string type, const mu_utf8string message) : DateTime(QDateTime::currentDateTime()), Type(type), Message(message) {}
    ~NLogMessage() {}

public:
    const QDateTime DateTime;
    const mu_utf8string Type;
    const mu_utf8string Message;
};

typedef std::unique_ptr<NLogMessage> NLogMessagePtr;

#endif
