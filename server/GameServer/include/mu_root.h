#ifndef __MU_ROOT_H__
#define __MU_ROOT_H__

#pragma once

class QCoreApplication;
class QQmlApplicationEngine;
class NRootContext;

namespace MURoot
{
    void Initialize(QCoreApplication *app, QQmlApplicationEngine *engine);
    NRootContext *GetRootContext();
};

#endif