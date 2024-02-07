#include "mu_precompiled.h"
#include "n_root_application.h"
#include "n_root_context.qml.h"
#include "mu_uievents.h"
#include "mu_root.h"

QEvent::Type NCustomEvent::Id = QEvent::None;

bool NApplication::event(QEvent *event)
{
    if (event->type() == NCustomEvent::GetId())
    {
        NCustomEvent *te = static_cast<NCustomEvent*>(event);

        switch(te->GetType())
        {
        case NEventType::UpdateFPS:
            {
                UIUpdateFPSEvent *e = static_cast<UIUpdateFPSEvent*>(te);
                const mu_utf8string performanceStatistics = fmt::format("{} FPS (Work : {:.2f} ms, Elapsed : {:.2f} ms)", e->FPS, e->WorkTime, e->ElapsedTime).c_str();
                MURoot::GetRootContext()->setPerformanceStatistics(performanceStatistics.c_str());
            }
            break;

        case NEventType::UpdateConsole:
            {
                auto *consoleLogger = MULogger::GetConsoleLogger();
                if (consoleLogger == nullptr) break;

                auto messages = consoleLogger->GetMessages();
                if (messages.empty() == false)
                {
                    auto *rootContext = MURoot::GetRootContext();
                    if (rootContext == nullptr) break;

                    constexpr qsizetype MaxConsoleLogs = 80;
                    QList<NConsoleMessage*> &currentMessages = rootContext->getMessagesList();
                    std::vector<NConsoleMessage*> messagesToDelete;

                    const auto currentMessagesCount = currentMessages.size();
                    const auto messagesToAddOffset = glm::max(0ll, static_cast<qsizetype>(messages.size()) - MaxConsoleLogs);
                    const auto messagesToAddCount = static_cast<qsizetype>(messages.size()) - messagesToAddOffset;
                    const auto mixedMessagesCount = messagesToAddCount + currentMessagesCount;
                    if (mixedMessagesCount > MaxConsoleLogs)
                    {
                        const auto messagesToRemoveCount = glm::min(currentMessagesCount, mixedMessagesCount - MaxConsoleLogs);
                        messagesToDelete.reserve(messagesToRemoveCount);
                        std::copy(currentMessages.begin() + (currentMessagesCount - messagesToRemoveCount), currentMessages.end(), std::back_inserter(messagesToDelete));

                        if (messagesToRemoveCount < MaxConsoleLogs)
                        {
                            for (auto dst_iter = currentMessages.rbegin(), src_iter = dst_iter + messagesToRemoveCount; src_iter != currentMessages.rend(); ++dst_iter, ++src_iter)
                            {
                                *dst_iter = *src_iter;
                            }
                        }
                    }
                    else // Grow currentMessages and move all messages to the end
                    {
                        currentMessages.resize(currentMessagesCount + messagesToAddCount, nullptr);
                        for (auto dst_iter = currentMessages.rbegin(), src_iter = dst_iter + messagesToAddCount; src_iter != currentMessages.rend(); ++dst_iter, ++src_iter)
                        {
                            *dst_iter = *src_iter;
                        }
                    }

                    // Assign Messages to currentMessages
                    {
                        auto dst_iter = currentMessages.begin();
                        auto src_iter = messages.begin() + messagesToAddOffset;
                        for (; src_iter != messages.end(); ++dst_iter, ++src_iter)
                        {
                            auto &m = *src_iter;
                            const QDate &date = m->DateTime.date();
                            const QTime &time = m->DateTime.time();
                            const QString message = fmt::format("[{:04}-{:02}-{:02} {:02}:{:02}:{:02}] {}", date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), m->Message).c_str();
                            QString backgroundColor, fontColor, selectedColor, highlightColor;

                            const auto *type = MULogger::GetType(m->Type);
                            if (type != nullptr)
                            {
                                backgroundColor = type->BackgroundColor;
                                fontColor = type->FontColor;
                                selectedColor = type->SelectedColor;
                                highlightColor = type->HighlightColor;
                            }

                            *dst_iter = new NConsoleMessage(message, backgroundColor, fontColor, selectedColor, highlightColor);
                        }
                    }

                    emit rootContext->messagesChanged();
                    for (auto message : messagesToDelete)
                    {
                        message->deleteLater();
                    }
                }
            }
            break;
        }

        return true;
    }

#if NEXTMU_CONSOLE_MODE == 1
    return QCoreApplication::event(event);
#else
    return QGuiApplication::event(event);
#endif
}
