#ifndef __SHARED_PRECOMPILED_H__
#define __SHARED_PRECOMPILED_H__

#pragma once

/* Fmt Library */
#define FMT_HEADER_ONLY (1)
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

#if NEXTMU_CLIENT_SHARED == 1
#include <SDL.h>
#else
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#endif

#include "shared_operatingsystem.h"

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
#include <windows.h>
#endif

#include "shared_standardtypes.h"
#include "shared_operatingsystem_io.h"
#include "shared_operatingsystem_backend.h"
#include "shared_objectid.h"
#include "shared_uuid.h"
#include "shared_objectid.h"
#include "shared_json.h"
#include "shared_crc32.h"
#include "shared_memorybuffer.h"
#include "shared_compression.h"

#include "shared_enums.h"

#endif