#include "app_environment.h"
#include "import_qml_components_plugins.h"
#include "import_qml_plugins.h"
#include "mu_root.h"
#include "n_root_application.h"
#include "n_root_context.qml.h"
#include <QIcon>

int main(int argc, char *argv[])
{
    set_qt_environment();

    NApplication app(argc, argv);

#if NEXTMU_CONSOLE_MODE == 1
    MURoot::Initialize(&app, nullptr);
#else
    app.setWindowIcon(QIcon(":/icon.ico"));

    QQmlApplicationEngine engine;
    qmlRegisterType<NConsoleMessage>("GameServerBackend", 1, 0, "NConsoleMessage");
    qmlRegisterType<NRootContext>("GameServerBackend", 1, 0, "NRootContext");

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

    MURoot::Initialize(&app, &engine);
#endif

    return app.exec();
}
