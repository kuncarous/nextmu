#include "mu_precompiled.h"
#include "mu_directories.h"

namespace MUDirectories
{
    mu_utf8string GameData = "../data/";
    mu_utf8string SharedData = "../data/";
    mu_utf8string ServerData = "data/";

    const mu_boolean Load()
    {
        return true;
    }

    const mu_utf8string GetGameData()
    {
        return GameData;
    }

    const mu_utf8string GetSharedData()
    {
        return SharedData;
    }

    const mu_utf8string GetServerData()
    {
        return ServerData;
    }
};