#include "mu_precompiled.h"
#include "mu_logger.h"
#include "n_root_context.h"
#include "n_logger_message.h"

NConsoleMessage::NConsoleMessage(const NLogMessage &message)
{
    const QDate &date = message.DateTime.date();
    const QTime &time = message.DateTime.time();
    Message = fmt::format("[{:04}-{:02}-{:02} {:02}:{:02}:{:02}] {}", date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), message.Message).c_str();

    const auto *type = MULogger::GetType(message.Type);
    if (type != nullptr)
    {
        BackgroundColor = type->BackgroundColor;
        FontColor = type->FontColor;
        SelectedColor = type->SelectedColor;
        HighlightColor = type->HighlightColor;
    }
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

QString NConsoleMessage::getMessage() const
{
    return Message;
}

NRootContext::~NRootContext()
{
    for (auto message : Messages)
    {
        delete message;
    }
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

QList<NConsoleMessage*> &NRootContext::getMessages()
{
    return Messages;
}

QQmlListProperty<NConsoleMessage> NRootContext::getQmlMessages()
{
    return QQmlListProperty<NConsoleMessage>(this, &Messages);
}
