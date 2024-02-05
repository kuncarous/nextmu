#ifndef __N_ROOT_THREAD_H__
#define __N_ROOT_THREAD_H__

#pragma once

#include <QThread>

class QCoreApplication;
class QQmlApplicationEngine;

class NRootThread : public QThread
{
public:
    void initialize(QCoreApplication *app, QQmlApplicationEngine *engine);
    void shutdown();

protected:
    virtual void run() override;

private:
    QCoreApplication *App = nullptr;
    QQmlApplicationEngine *Engine = nullptr;
    mu_atomic_bool ShouldShutdown = false;
};

#endif
