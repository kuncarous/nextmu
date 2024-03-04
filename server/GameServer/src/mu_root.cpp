#include "mu_precompiled.h"
#include "mu_root.h"
#include "n_root_thread.h"
#include "n_root_context.qml.h"
#include "mu_uievents.h"

namespace MURoot
{
    QCoreApplication *App = nullptr;
    QQmlApplicationEngine *Engine = nullptr;
    NRootContext *RootContext = nullptr;
    QTimer *ConsoleTimer = nullptr;
    std::unique_ptr<NRootThread> RootThread;

    void AboutToCloseEvent();
    void Initialize(QCoreApplication *app, QQmlApplicationEngine *engine)
    {
        App = app;
        Engine = engine;

        auto objects = Engine->rootObjects();
        if (objects.empty())
        {
            return;
        }

        RootContext = objects.first()->findChild<NRootContext*>("rootCxt");
        ConsoleTimer = new_nothrow QTimer(App);
        ConsoleTimer->setInterval(500);
        ConsoleTimer->setTimerType(Qt::PreciseTimer);
        ConsoleTimer->callOnTimeout(
            []() {
                App->postEvent(App, new UIUpdateConsoleEvent());
            }
        );
        ConsoleTimer->start();

        QObject::connect(app, &QCoreApplication::aboutToQuit, AboutToCloseEvent);
        RootThread.reset(new_nothrow NRootThread());
        if (RootThread == nullptr)
        {
            return;
        }

        RootThread->initialize(app, engine);
        RootThread->start();
    }

    void Destroy()
    {
        if (RootThread != nullptr)
        {
            RootThread->shutdown();
            RootThread.reset();
        }
    }

    NRootContext *GetRootContext()
    {
        return RootContext;
    }

    void AboutToCloseEvent()
    {
        Destroy();
    }
};
