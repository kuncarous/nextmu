#ifndef __MU_DIRECTORIES_H__
#define __MU_DIRECTORIES_H__

#pragma once

namespace MUDirectories
{
    const mu_boolean Load();
    const mu_utf8string GetGameData();
    const mu_utf8string GetServerData();
    const mu_utf8string GetSharedData();
};

#endif
