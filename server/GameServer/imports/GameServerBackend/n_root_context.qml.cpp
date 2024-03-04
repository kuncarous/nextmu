#include "n_root_context.qml.h"

NConsoleMessage::NConsoleMessage(QObject *parent) : QObject(parent) {}

NConsoleMessage::NConsoleMessage(
    const QString message,
    const QString backgroundColor,
    const QString fontColor,
    const QString selectedColor,
    const QString highlightColor,
    QObject *parent
) :
    QObject(parent),
    Message(message),
    BackgroundColor(backgroundColor),
    FontColor(fontColor),
    SelectedColor(selectedColor),
    HighlightColor(highlightColor)
{}

QString NConsoleMessage::getMessage() const
{
    return Message;
}

QString NConsoleMessage::getBackgroundColor() const
{
    return BackgroundColor;
}

QString NConsoleMessage::getFontColor() const
{
    return FontColor;
}

QString NConsoleMessage::getSelectedColor() const
{
    return SelectedColor;
}

QString NConsoleMessage::getHighlightColor() const
{
    return HighlightColor;
}

void NConsoleMessage::setMessage(const QString &value)
{
    Message = value;
}

void NConsoleMessage::setBackgroundColor(const QString &value)
{
    BackgroundColor = value;
}

void NConsoleMessage::setFontColor(const QString &value)
{
    FontColor = value;
}

void NConsoleMessage::setSelectedColor(const QString &value)
{
    SelectedColor = value;
}

void NConsoleMessage::setHighlightColor(const QString &value)
{
    HighlightColor = value;
}

QString NRootContext::getPerformanceStatistics() const
{
    return PerformanceStatistics;
}

void NRootContext::setPerformanceStatistics(const QString &value)
{
    PerformanceStatistics = value;
    emit performanceStatisticsChanged();
}

QQmlListProperty<NConsoleMessage> NRootContext::getMessages()
{
    return QQmlListProperty<NConsoleMessage>(this, &Messages);
}

QList<NConsoleMessage*> &NRootContext::getMessagesList()
{
    return Messages;
}
