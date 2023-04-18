#ifndef __MU_PRECOMPILED_H__
#define __MU_PRECOMPILED_H__

#pragma once

// This doesn't change only the layout of glm::quat so shouldn't be used
//#define GLM_FORCE_QUAT_DATA_XYZW 1
#define GLM_FORCE_QUAT_DATA_WXYZ 1 // vcpkg glm requires this to work as before

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
namespace cglm
{
#include <cglm/types.h>
#include <cglm/cam.h>
#include <cglm/frustum.h>
#include <cglm/box.h>
#include <cglm/sphere.h>
#include <cglm/clipspace/persp_rh_zo.h>
}

#include <SDL.h>
#include <bgfx/bgfx.h>
#include <FreeImage.h>
#include <entt/entt.hpp>
#include <angelscript.h>

#if PHYSICS_ENABLED == 1
#include <PxPhysicsAPI.h>
#endif

#include "shared_precompiled.h"
#include "mu_version.h"

#define NEXTMU_TITLE "NextMU Project"

#define NEXTMU_UI_DUMMY (0)
#define NEXTMU_UI_NOESISGUI (1)
#define NEXTMU_UI_LIBRARY NEXTMU_UI_NOESISGUI

#define NEXTMU_COMPRESSED_PARTICLES (0)
#define NEXTMU_COMPRESSED_JOINTS (0)

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
/*
	If this file is missing means you have to create it,
	use ui_noesisgui_license.h.template as base,
	put your license information and you're ready to use NoesisGUI.
*/
#include "ui_noesisgui_license.h"
#include <NoesisPCH.h>
#endif

#include "mu_math.h"
#include "mu_texture.h"
#include "mu_model.h"
#include "mu_terrain.h"
#include "mu_skeletoninstance.h"

#endif