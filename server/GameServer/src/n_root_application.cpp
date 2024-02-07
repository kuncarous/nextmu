#include "mu_precompiled.h"
#include "n_root_application.h"
#include "n_root_context.qml.h"

NApplication::NApplication(int &argc, char **argv) :
#if NEXTMU_CONSOLE_MODE == 1
    QCoreApplication(argc, argv)
#else
    QGuiApplication(argc, argv)
#endif
{

}