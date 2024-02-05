#include "mu_precompiled.h"
#include "mu_root.h"
#include "n_root_thread.h"
#include "n_root_context.h"

#if NEXTMU_CONSOLE_MODE == 1
#include <QCoreApplication>
#else
#include <QGuiApplication>
#endif
#include <QQmlApplicationEngine>
#include <QQmlContext>

namespace MURoot
{
    QCoreApplication *App = nullptr;
    QQmlApplicationEngine *Engine = nullptr;
    std::unique_ptr<NRootContext> RootContext;
    std::unique_ptr<NRootThread> RootThread;

    void AboutToCloseEvent();
    void Initialize(QCoreApplication *app, QQmlApplicationEngine *engine)
    {
        App = app;
        Engine = engine;
        RootContext.reset(new_nothrow NRootContext());
        if (engine != nullptr)
        {
            engine->rootContext()->setContextProperty("rootCxt", RootContext.get());
        }

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
        return RootContext.get();
    }

    void AboutToCloseEvent()
    {
        Destroy();
    }
};
