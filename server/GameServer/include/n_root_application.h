#ifndef __N_ROOT_APPLICATION_H__
#define __N_ROOT_APPLICATION_H__

#pragma once

#if NEXTMU_CONSOLE_MODE == 1
#include <QCoreApplication>
#else
#include <QGuiApplication>
#endif
#include <QQmlApplicationEngine>
#include <QQmlContext>

class NRootContext;

class NApplication :
#if NEXTMU_CONSOLE_MODE == 1
    public QCoreApplication
#else
    public QGuiApplication
#endif
{
public:
    NApplication(int &argc, char **argv);

protected:
    bool event(QEvent *event) override;
};

#endif