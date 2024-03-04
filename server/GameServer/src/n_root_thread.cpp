#include "mu_precompiled.h"
#include "n_root_thread.h"
#include "mu_threadsmanager.h"
#include "mu_resourcesmanager.h"
#include "mu_timer.h"
#include "mu_uievents.h"
#include <QCoreApplication>
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
    MUThreadsManager::Initialize();

    /*if (MUResourcesManager::Load() == false)
    {
        App->postEvent(App, new UIUpdateConsoleEvent());
        App->quit();
        return;
    }*/

    MUGlobalTimer::Wait();
    mu_double workTime = 0.0;
    mu_double elapsedTime = 0.0;
    mu_double currentTime = 0.0;

    mu_uint32 frameCount = 0u;
    mu_double accWorkTime = 0.0;
    mu_double accElapsedTime = 0.0;

    while (true)
    {
        if (ShouldShutdown)
        {
            break;
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
            App->postEvent(App, new UIUpdateFPSEvent(frameCount, accWorkTime * avgDivider, accElapsedTime * avgDivider));

            frameCount = 0u;
            accWorkTime = 0.0;
            accElapsedTime = 0.0;
        }
    }

    MULogger::Destroy();
    MUThreadsManager::Destroy();

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
    timeEndPeriod(1);
#endif
}
