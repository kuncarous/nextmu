#ifndef __MU_PRECOMPILED_H__
#define __MU_PRECOMPILED_H__

#pragma once

// This doesn't change only the layout of glm::quat so shouldn't be used
//#define GLM_FORCE_QUAT_DATA_XYZW 1

#include <SDL.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <FreeImage.h>
#include <entt/entt.hpp>

#include "shared_precompiled.h"
#include "mu_version.h"

#define NEXTMU_TITLE "NextMU Project"

#define NEXTMU_UI_IMGUI (0) // Windows only
#define NEXTMU_UI_ULTRALIGHT (1) // Desktop only
#define NEXTMU_UI_NOESISGUI (2)

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
#define NEXTMU_UI_LIBRARY NEXTMU_UI_NOESISGUI
#else
#define NEXTMU_UI_LIBRARY NEXTMU_UI_IMGUI
//#define NEXTMU_UI_LIBRARY NEXTMU_UI_NOESISGUI
#endif

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
/*
	If this file is missing means you have to create it,
	use ui_noesisgui_license.h.template as base,
	put your license information and you're ready to use NoesisGUI.
*/
#include "ui_noesisgui_license.h"
#include <NoesisPCH.h>
#endif

#include "mu_texture.h"
#include "mu_model.h"
#include "mu_terrain.h"
#include "mu_skeletoninstance.h"

#endif