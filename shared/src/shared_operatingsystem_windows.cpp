#include "shared_precompiled.h"

void ConsoleDebugPrint(const mu_char *message)
{
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
    OutputDebugStringA(message);
#endif
}