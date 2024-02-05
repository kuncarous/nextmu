#include "mu_precompiled.h"
#include "n_root_thread.h"
#include "n_root_context.h"
#include "mu_root.h"
#include "mu_timer.h"
#include "mu_logger.h"

#if NEXTMU_CONSOLE_MODE == 1
#include <QCoreApplication>
#else
#include <QGuiApplication>
#endif
#include <QQmlApplicationEngine>

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")
#endif

void NRootThread::initialize(QCoreApplication *app, QQmlApplicationEngine *engine)
{
    App = app;
    Engine = engine;
}

void NRootThread::shutdown()
{
    ShouldShutdown = true;
    this->wait();
}

void NRootThread::run()
{
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
    // Ask windows to have 1 millisecond of precision in timer resolution
    timeBeginPeriod(1);
#endif

    MULogger::Initialize();

    MUGlobalTimer::Wait();
    mu_double workTime = 0.0;
    mu_double elapsedTime = 0.0;
    mu_double currentTime = 0.0;

    mu_uint32 frameCount = 0u;
    mu_double accWorkTime = 0.0;
    mu_double accElapsedTime = 0.0;

    auto *consoleLogger = MULogger::GetConsoleLogger();
    auto *rootContext = MURoot::GetRootContext();

    MULogger::LogToConsole("message", "testing a message");
    MULogger::LogToConsole("warning", "testing a warning");
    MULogger::LogToConsole("error", "testing an error");

    while (true)
    {
        if (ShouldShutdown)
        {
            break;
        }

        // Update Console
        {
            const auto messages = consoleLogger->GetMessages();
            if (messages.empty() == false)
            {
                constexpr qsizetype MaxConsoleLogs = 200;
                QList<NConsoleMessage*> &currentMessages = rootContext->getMessages();
                QList<NConsoleMessage*> messagesToDelete;

                const auto mixedMessagesCount = static_cast<qsizetype>(messages.size()) + currentMessages.size();
                if (mixedMessagesCount > MaxConsoleLogs)
                {
                    const auto countToRemove = glm::min(currentMessages.size(), mixedMessagesCount - MaxConsoleLogs);
                    std::copy(currentMessages.begin(), currentMessages.begin() + countToRemove, std::back_inserter(messagesToDelete));
                    currentMessages.remove(0, glm::min(currentMessages.size(), mixedMessagesCount - MaxConsoleLogs));
                }

                for (const auto &message : messages)
                {
                    currentMessages.append(new NConsoleMessage(message));
                }

                emit rootContext->messagesChanged();
                for (auto message : messagesToDelete)
                {
                    delete message;
                }
            }
        }

        MUGlobalTimer::Wait();
        workTime = MUGlobalTimer::GetWorkFrametime();
        elapsedTime = MUGlobalTimer::GetElapsedFrametime();
        currentTime = MUGlobalTimer::GetFrametime();

        ++frameCount;
        accWorkTime += workTime;
        accElapsedTime += elapsedTime;

        if (accElapsedTime >= 1000.0)
        {
            const mu_double avgDivider = 1.0 / static_cast<mu_double>(frameCount);
            const mu_utf8string performanceStatistics = fmt::format("{} FPS (Work : {:.2f} ms, Elapsed : {:.2f} ms)", frameCount, accWorkTime * avgDivider, accElapsedTime * avgDivider).c_str();

#if NEXTMU_CONSOLE_MODE == 1
#else
            rootContext->setPerformanceStatistics(performanceStatistics.c_str());
#endif

            frameCount = 0u;
            accWorkTime = 0.0;
            accElapsedTime = 0.0;
        }
    }

    MULogger::Destroy();

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
    timeEndPeriod(1);
#endif
}
