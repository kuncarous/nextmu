// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if NEXTMU_CONSOLE_MODE == 1
#include <QCoreApplication>
#else
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#endif

#include "app_environment.h"
#include "import_qml_components_plugins.h"
#include "import_qml_plugins.h"
#include "mu_root.h"

int main(int argc, char *argv[])
{
    set_qt_environment();

#if NEXTMU_CONSOLE_MODE == 1
    QCoreApplication app(argc, argv);
    MURoot::Initialize(&app, nullptr);
#else
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    MURoot::Initialize(&app, &engine);

    const QUrl url(u"qrc:/qt/qml/Main/main.qml"_qs);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
#endif

    return app.exec();
}
