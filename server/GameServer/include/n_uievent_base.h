#ifndef __N_UIEVENT_BASE_H__
#define __N_UIEVENT_BASE_H__

#pragma once

#include <QEvent>

enum class NEventType
{
    UpdateFPS,
    UpdateConsole,
};

class NCustomEvent : public QEvent
{
public:
    NCustomEvent(const NEventType eventType) : QEvent(registeredType()), EventType(eventType) {}

public:
    static const QEvent::Type GetId()
    {
        return Id;
    }

    const NEventType GetType() const
    {
        return EventType;
    }

private:
    const NEventType EventType;

private:
    static QEvent::Type Id;
    static QEvent::Type registeredType()
    {
        if (Id == QEvent::None) Id = static_cast<QEvent::Type>(QEvent::registerEventType());
        return Id;
    }
};

#endif
