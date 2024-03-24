#ifndef __MU_PRECOMPILED_H__
#define __MU_PRECOMPILED_H__

#pragma once

#include "shared_precompiled.h"

// This doesn't change only the layout of glm::quat so shouldn't be used
//#define GLM_FORCE_QUAT_DATA_XYZW 1
#define GLM_FORCE_QUAT_DATA_WXYZ 1 // vcpkg glm requires this to work as before
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <FreeImage.h>
#include <entt/entt.hpp>
#include <angelscript.h>
typedef std::shared_ptr<AngelScript::asIScriptModule> ASModuleScript;

#if PHYSICS_ENABLED == 1
#include <PxPhysicsAPI.h>
#endif

#include "mu_version.h"

/* Diligent Engine */
#include <RefCntAutoPtr.hpp>
#include <EngineFactory.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <ScreenCapture.hpp>
#include <GraphicsTypesX.hpp>
#include <AdvancedMath.hpp>
#include <ShadowMapManager.hpp>

#define NEXTMU_TITLE "NextMU Project"

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS || \
    NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX || \
    NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
#define NEXTMU_EMBEDDED_BROWSER (1)
#define NEXTMU_HTTP_SERVER		(1)
#else
#define NEXTMU_EMBEDDED_BROWSER (0)
#define NEXTMU_HTTP_SERVER		(0)
#endif

#define NEXTMU_COMPRESSED_MESHS (0)
#define NEXTMU_COMPRESSED_PARTICLES (0)
#define NEXTMU_COMPRESSED_JOINTS (0)

#define NEXTMU_RENDER_BBOX (0)

/*
	If this file is missing means you have to create it,
	use ui_noesisgui_license.h.template as base,
	put your license information and you're ready to use NoesisGUI.
*/
#include "ui_noesisgui_license.h"
#include <NoesisPCH.h>
#define NS_APP_INTERACTIVITY_API NS_DLL_LOCAL
#define NS_APP_MEDIAELEMENT_API NS_DLL_LOCAL

#include "mu_math.h"
#include "mu_model.h"
#include "mu_terrain.h"
#include "mu_skeletoninstance.h"

#endif