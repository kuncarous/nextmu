#ifndef __SHARED_PRECOMPILED_H__
#define __SHARED_PRECOMPILED_H__

#pragma once

/* Fmt Library */
#define FMT_HEADER_ONLY (1)
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

#include <SDL.h>

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
#include <windows.h>
#endif

#include "shared_operatingsystem.h"
#include "shared_standardtypes.h"
#include "shared_operatingsystem_io.h"
#include "shared_operatingsystem_backend.h"
#include "shared_json.h"

#include "shared_enums.h"

#endif