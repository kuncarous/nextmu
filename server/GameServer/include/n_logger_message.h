#ifndef __N_LOGGER_MESSAGE_H__
#define __N_LOGGER_MESSAGE_H__

#pragma once

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

#endif