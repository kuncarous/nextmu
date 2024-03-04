# Root Group
set(
    G_ROOT_INCLUDE
	include/mu_root.h
)

set(
    G_ROOT_SOURCE
	src/mu_root.cpp
)

source_group("Root" FILES ${G_ROOT_SOURCE} ${G_ROOT_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ROOT_INCLUDE})
list(APPEND GAME_SOURCE ${G_ROOT_SOURCE})

# Version Group
set(
    G_VERSION_INCLUDE
	include/mu_version.h
)

source_group("Version" FILES ${G_VERSION_INCLUDE})
list(APPEND GAME_INCLUDE ${G_VERSION_INCLUDE})

# Window Group
set(
    G_WINDOW_INCLUDE
	include/mu_window.h
)

set(
    G_WINDOW_SOURCE
	src/mu_window.cpp
)

source_group("Window" FILES ${G_WINDOW_SOURCE} ${G_WINDOW_INCLUDE})
list(APPEND GAME_INCLUDE ${G_WINDOW_INCLUDE})
list(APPEND GAME_SOURCE ${G_WINDOW_SOURCE})

# UI NoesisGUI Group
set(
    G_UI_NOESISGUI_INCLUDE
	include/ui_noesisgui_consts.h
	include/ui_noesisgui_fontprovider.h
	include/ui_noesisgui_renderdevice.h
	include/ui_noesisgui_rendertarget.h
	include/ui_noesisgui_sdl.h
	include/ui_noesisgui_stream.h
	include/ui_noesisgui_texture.h
	include/ui_noesisgui_textureprovider.h
	include/ui_noesisgui_xamlprovider.h
	include/ui_noesisgui.h
)

set(
    G_UI_NOESISGUI_SOURCE
	src/ui_noesisgui_consts.cpp
	src/ui_noesisgui_fontprovider.cpp
	src/ui_noesisgui_renderdevice.cpp
	src/ui_noesisgui_rendertarget.cpp
	src/ui_noesisgui_sdl.cpp
	src/ui_noesisgui_stream.cpp
	src/ui_noesisgui_texture.cpp
	src/ui_noesisgui_textureprovider.cpp
	src/ui_noesisgui_xamlprovider.cpp
	src/ui_noesisgui.cpp
)

source_group("UI\\NoesisGUI" FILES ${G_UI_NOESISGUI_SOURCE} ${G_UI_NOESISGUI_INCLUDE})
list(APPEND GAME_INCLUDE ${G_UI_NOESISGUI_INCLUDE})
list(APPEND GAME_SOURCE ${G_UI_NOESISGUI_SOURCE})

# NoesisGUI Classes Group
set(
    G_NOESISGUI_CLASSES_INCLUDE
	include/ngui_notifier.h
)

set(
    G_NOESISGUI_CLASSES_SOURCE
	src/ngui_notifier.cpp
)

source_group("UI\\NoesisGUI\\Classes" FILES ${G_NOESISGUI_CLASSES_SOURCE} ${G_NOESISGUI_CLASSES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_NOESISGUI_CLASSES_INCLUDE})
list(APPEND GAME_SOURCE ${G_NOESISGUI_CLASSES_SOURCE})

# NoesisGUI Contexts Group
set(
    G_NG_CONTEXTS_INCLUDE
	include/ngui_context.h
)

set(
    G_NG_CONTEXTS_SOURCE
	src/ngui_context.cpp
)

source_group("UI\\NoesisGUI\\Contexts" FILES ${G_NG_CONTEXTS_SOURCE} ${G_NG_CONTEXTS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_NG_CONTEXTS_INCLUDE})
list(APPEND GAME_SOURCE ${G_NG_CONTEXTS_SOURCE})

# NoesisGUI Contexts Update Group
set(
    G_NG_CONTEXTS_UPDATE_INCLUDE
	include/ngui_context_update.h
)

set(
    G_NG_CONTEXTS_UPDATE_SOURCE
	src/ngui_context_update.cpp
)

source_group("UI\\NoesisGUI\\Contexts\\Update" FILES ${G_NG_CONTEXTS_UPDATE_SOURCE} ${G_NG_CONTEXTS_UPDATE_INCLUDE})
list(APPEND GAME_INCLUDE ${G_NG_CONTEXTS_UPDATE_INCLUDE})
list(APPEND GAME_SOURCE ${G_NG_CONTEXTS_UPDATE_SOURCE})

# UI NoesisGUI Extensions Group
if(UI_LIBRARY STREQUAL "NoesisGUI")
    # RichText
    set(
        G_NG_EXTENSIONS_RICHTEXT_INCLUDE
        noesis/richtext/RichText.h
    )

    set(
        G_NG_EXTENSIONS_RICHTEXT_SOURCE
        noesis/richtext/RichText.cpp
    )

    source_group("UI\\NoesisGUI\\Extensions\\RichText" FILES ${G_NG_EXTENSIONS_RICHTEXT_INCLUDE} ${G_NG_EXTENSIONS_RICHTEXT_SOURCE})
    list(APPEND GAME_INCLUDE ${G_NG_EXTENSIONS_RICHTEXT_SOURCE})
    list(APPEND GAME_SOURCE ${G_NG_EXTENSIONS_RICHTEXT_INCLUDE})

    # Localization
    set(
        G_NG_EXTENSIONS_LOCALIZATION_INCLUDE
        noesis/localization/LocExtension.h
    )

    set(
        G_NG_EXTENSIONS_LOCALIZATION_SOURCE
        noesis/localization/LocExtension.cpp
    )

    source_group("UI\\NoesisGUI\\Extensions\\Localization" FILES ${G_NG_EXTENSIONS_LOCALIZATION_INCLUDE} ${G_NG_EXTENSIONS_LOCALIZATION_SOURCE})
    list(APPEND GAME_INCLUDE ${G_NG_EXTENSIONS_LOCALIZATION_SOURCE})
    list(APPEND GAME_SOURCE ${G_NG_EXTENSIONS_LOCALIZATION_INCLUDE})
endif()

# UI RmlUI Group
set(
    G_UI_RMLUI_INCLUDE
	include/ui_rmlui.h
    include/ui_rmlui_system.h
    include/ui_rmlui_renderer.h
)

set(
    G_UI_RMLUI_SOURCE
	src/ui_rmlui.cpp
    src/ui_rmlui_system.cpp
    src/ui_rmlui_renderer.cpp
)

source_group("UI\\RmlUI" FILES ${G_UI_RMLUI_SOURCE} ${G_UI_RMLUI_INCLUDE})
list(APPEND GAME_INCLUDE ${G_UI_RMLUI_INCLUDE})
list(APPEND GAME_SOURCE ${G_UI_RMLUI_SOURCE})

# Navigation Group
set(
    G_NAVIGATION_INCLUDE
	include/mu_navigation.h
	include/nav_path.h
	include/nav_polys.h
)

set(
    G_NAVIGATION_SOURCE
	src/mu_navigation.cpp
)

source_group("Navigation" FILES ${G_NAVIGATION_SOURCE} ${G_NAVIGATION_INCLUDE})
list(APPEND GAME_INCLUDE ${G_NAVIGATION_INCLUDE})
list(APPEND GAME_SOURCE ${G_NAVIGATION_SOURCE})

# Navigation Detour
set(
    G_NAVIGATION_DETOUR_INCLUDE
    Detour/Include/DetourAlloc.h
    Detour/Include/DetourAssert.h
    Detour/Include/DetourCommon.h
    Detour/Include/DetourMath.h
    Detour/Include/DetourNavMesh.h
    Detour/Include/DetourNavMeshBuilder.h
    Detour/Include/DetourNavMeshQuery.h
    Detour/Include/DetourNode.h
    Detour/Include/DetourStatus.h
)

set(
    G_NAVIGATION_DETOUR_SOURCE
    Detour/Source/DetourAlloc.cpp
    Detour/Source/DetourAssert.cpp
    Detour/Source/DetourCommon.cpp
    Detour/Source/DetourNavMesh.cpp
    Detour/Source/DetourNavMeshBuilder.cpp
    Detour/Source/DetourNavMeshQuery.cpp
    Detour/Source/DetourNode.cpp
)

source_group("Navigation\\Detour" FILES ${G_NAVIGATION_DETOUR_SOURCE} ${G_NAVIGATION_DETOUR_INCLUDE})
list(APPEND GAME_INCLUDE ${G_NAVIGATION_DETOUR_INCLUDE})
list(APPEND GAME_SOURCE ${G_NAVIGATION_DETOUR_SOURCE})

# Animations Group
set(
    G_ANIMATIONS_INCLUDE
	include/ani_input.h
	include/ani_node.h
	include/mu_animationsmanager.h
)

set(
    G_ANIMATIONS_SOURCE
	src/mu_animationsmanager.cpp
)

source_group("Animations" FILES ${G_ANIMATIONS_SOURCE} ${G_ANIMATIONS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ANIMATIONS_INCLUDE})
list(APPEND GAME_SOURCE ${G_ANIMATIONS_SOURCE})

# Attachments Group
set(
    G_ATTACHMENTS_INCLUDE
	include/t_textureattachments.h
)

source_group("Attachments" FILES  ${G_ATTACHMENTS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ATTACHMENTS_INCLUDE})

# Camera Group
set(
    G_CAMERA_INCLUDE
	include/mu_camera.h
)

set(
    G_CAMERA_SOURCE
	src/mu_camera.cpp
)

source_group("Camera" FILES ${G_CAMERA_SOURCE} ${G_CAMERA_INCLUDE})
list(APPEND GAME_INCLUDE ${G_CAMERA_INCLUDE})
list(APPEND GAME_SOURCE ${G_CAMERA_SOURCE})

# Caps Group
set(
    G_CAPS_INCLUDE
	include/mu_capabilities.h
)

set(
    G_CAPS_SOURCE
	src/mu_capabilities.cpp
)

source_group("Caps" FILES ${G_CAPS_SOURCE} ${G_CAPS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_CAPS_INCLUDE})
list(APPEND GAME_SOURCE ${G_CAPS_SOURCE})

# Characters Group
set(
    G_CHARACTERS_INCLUDE
	include/mu_charactersmanager.h
	include/t_charactersmanager_structs.h
)

set(
    G_CHARACTERS_SOURCE
	src/mu_charactersmanager.cpp
)

source_group("Characters" FILES ${G_CHARACTERS_SOURCE} ${G_CHARACTERS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_CHARACTERS_INCLUDE})
list(APPEND GAME_SOURCE ${G_CHARACTERS_SOURCE})

# Config Group
set(
    G_CONFIG_INCLUDE
	include/mu_config.h
)

set(
    G_CONFIG_SOURCE
	src/mu_config.cpp
)

source_group("Config" FILES ${G_CONFIG_SOURCE} ${G_CONFIG_INCLUDE})
list(APPEND GAME_INCLUDE ${G_CONFIG_INCLUDE})
list(APPEND GAME_SOURCE ${G_CONFIG_SOURCE})

# Graphics Group
set(
    G_GRAPHICS_INCLUDE
	include/mu_graphics.h
	include/t_graphics.h
	include/t_graphics_resourceid.h
)

set(
    G_GRAPHICS_SOURCE
	src/mu_graphics.cpp
	src/t_graphics.cpp
	src/t_graphics_resourceid.cpp
)

source_group("Graphics" FILES ${G_GRAPHICS_SOURCE} ${G_GRAPHICS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_SOURCE})

# Graphics Layouts Group
set(
    G_GRAPHICS_LAYOUTS_INCLUDE
	include/t_graphics_layouts.h
)

set(
    G_GRAPHICS_LAYOUTS_SOURCE
	src/t_graphics_layouts.cpp
)

source_group("Graphics\\Layouts" FILES ${G_GRAPHICS_LAYOUTS_SOURCE} ${G_GRAPHICS_LAYOUTS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_LAYOUTS_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_LAYOUTS_SOURCE})

# Graphics Pipelines Group
set(
    G_GRAPHICS_PIPELINES_INCLUDE
	include/t_graphics_pipelineresources.h
	include/t_graphics_pipelines.h
	include/t_graphics_pipelinestate.h
)

set(
    G_GRAPHICS_PIPELINES_SOURCE
	src/t_graphics_pipelineresources.cpp
	src/t_graphics_pipelines.cpp
	src/t_graphics_pipelinestate.cpp
)

source_group("Graphics\\Pipelines" FILES ${G_GRAPHICS_PIPELINES_SOURCE} ${G_GRAPHICS_PIPELINES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_PIPELINES_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_PIPELINES_SOURCE})

# Graphics Renderer Group
set(
    G_GRAPHICS_RENDERER_INCLUDE
	include/t_graphics_renderclassifier.h
	include/t_graphics_rendermanager.h
	include/t_graphics_rendersettings.h
)

set(
    G_GRAPHICS_RENDERER_SOURCE
	src/t_graphics_renderclassifier.cpp
	src/t_graphics_rendermanager.cpp
)

source_group("Graphics\\Renderer" FILES ${G_GRAPHICS_RENDERER_SOURCE} ${G_GRAPHICS_RENDERER_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_RENDERER_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_RENDERER_SOURCE})

# Graphics Resources Group
set(
    G_GRAPHICS_RESOURCES_INCLUDE
	include/t_graphics_buffer.h
	include/t_graphics_texture.h
	include/t_graphics_resources.h
)

set(
    G_GRAPHICS_RESOURCES_SOURCE
	src/t_graphics_buffer.cpp
	src/t_graphics_texture.cpp
	src/t_graphics_resources.cpp
)

source_group("Graphics\\Resources" FILES ${G_GRAPHICS_RESOURCES_SOURCE} ${G_GRAPHICS_RESOURCES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_RESOURCES_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_RESOURCES_SOURCE})

# Graphics Samplers Group
set(
    G_GRAPHICS_SAMPLERS_INCLUDE
	include/t_graphics_immutables.h
	include/t_graphics_samplers.h
)

set(
    G_GRAPHICS_SAMPLERS_SOURCE
	src/t_graphics_immutables.cpp
	src/t_graphics_samplers.cpp
)

source_group("Graphics\\Samplers" FILES ${G_GRAPHICS_SAMPLERS_SOURCE} ${G_GRAPHICS_SAMPLERS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_SAMPLERS_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_SAMPLERS_SOURCE})

# Graphics Shaders Group
set(
    G_GRAPHICS_SHADERS_INCLUDE
	include/t_graphics_shader.h
	include/t_graphics_shaderresources.h
)

set(
    G_GRAPHICS_SHADERS_SOURCE
	src/t_graphics_shader.cpp
	src/t_graphics_shaderresources.cpp
)

source_group("Graphics\\Shaders" FILES ${G_GRAPHICS_SHADERS_SOURCE} ${G_GRAPHICS_SHADERS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_SHADERS_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_SHADERS_SOURCE})

# Graphics Shadows Group
set(
    G_GRAPHICS_SHADOWS_INCLUDE
	include/t_graphics_shadows.h
)

set(
    G_GRAPHICS_SHADOWS_SOURCE
	src/t_graphics_shadows.cpp
)

source_group("Graphics\\Shadows" FILES ${G_GRAPHICS_SHADOWS_SOURCE} ${G_GRAPHICS_SHADOWS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_GRAPHICS_SHADOWS_INCLUDE})
list(APPEND GAME_SOURCE ${G_GRAPHICS_SHADOWS_SOURCE})

# Renderer Group
set(
    G_RENDERER_INCLUDE
	include/mu_bboxrenderer.h
	include/mu_modelrenderer.h
	include/mu_rendererconfig.h
)

set(
    G_RENDERER_SOURCE
	src/mu_bboxrenderer.cpp
	src/mu_modelrenderer.cpp
)

source_group("Renderer" FILES ${G_RENDERER_SOURCE} ${G_RENDERER_INCLUDE})
list(APPEND GAME_INCLUDE ${G_RENDERER_INCLUDE})
list(APPEND GAME_SOURCE ${G_RENDERER_SOURCE})

# Renders Group
set(
    G_RENDERS_INCLUDE
	include/res_render.h
	include/res_renders.h
)

set(
    G_RENDERS_SOURCE
	src/res_renders.cpp
)

source_group("Renders" FILES ${G_RENDERS_SOURCE} ${G_RENDERS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_RENDERS_INCLUDE})
list(APPEND GAME_SOURCE ${G_RENDERS_SOURCE})

# Input Group
set(
    G_INPUT_INCLUDE
	include/mu_input.h
)

set(
    G_INPUT_SOURCE
	src/mu_input.cpp
)

source_group("Input" FILES ${G_INPUT_SOURCE} ${G_INPUT_INCLUDE})
list(APPEND GAME_INCLUDE ${G_INPUT_INCLUDE})
list(APPEND GAME_SOURCE ${G_INPUT_SOURCE})

# Physics Group
set(
    G_PHYSICS_INCLUDE
	include/mu_physics.h
)

set(
    G_PHYSICS_SOURCE
	src/mu_physics.cpp
)

source_group("Physics" FILES ${G_PHYSICS_SOURCE} ${G_PHYSICS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_PHYSICS_INCLUDE})
list(APPEND GAME_SOURCE ${G_PHYSICS_SOURCE})

# Threading Group
set(
    G_THREADING_INCLUDE
	include/mu_threadsmanager.h
	include/t_threading_helper.h
)

set(
    G_THREADING_SOURCE
	src/mu_threadsmanager.cpp
	src/t_threading_helper.cpp
)

source_group("Threading" FILES ${G_THREADING_SOURCE} ${G_THREADING_INCLUDE})
list(APPEND GAME_INCLUDE ${G_THREADING_INCLUDE})
list(APPEND GAME_SOURCE ${G_THREADING_SOURCE})

# Timer Group
set(
    G_TIMER_INCLUDE
	include/mu_timer.h
)

set(
    G_TIMER_SOURCE
	src/mu_timer.cpp
)

source_group("Timer" FILES ${G_TIMER_SOURCE} ${G_TIMER_INCLUDE})
list(APPEND GAME_INCLUDE ${G_TIMER_INCLUDE})
list(APPEND GAME_SOURCE ${G_TIMER_SOURCE})

# Utils Group
set(
    G_UTILS_INCLUDE
	include/mu_crypt.h
	include/mu_resizablequeue.h
)

set(
    G_UTILS_SOURCE
	src/mu_crypt.cpp
)

source_group("Utils" FILES ${G_UTILS_SOURCE} ${G_UTILS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_UTILS_INCLUDE})
list(APPEND GAME_SOURCE ${G_UTILS_SOURCE})

# Resources Group
set(
    G_RESOURCES_INCLUDE
	include/mu_resources.h
	include/mu_resourcesmanager.h
)

set(
    G_RESOURCES_SOURCE
	src/mu_resourcesmanager.cpp
)

source_group("Resources" FILES ${G_RESOURCES_SOURCE} ${G_RESOURCES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_RESOURCES_INCLUDE})
list(APPEND GAME_SOURCE ${G_RESOURCES_SOURCE})

# Textures Group
set(
    G_TEXTURES_INCLUDE
	include/mu_textures.h
)

set(
    G_TEXTURES_SOURCE
	src/mu_textures.cpp
)

source_group("Textures" FILES ${G_TEXTURES_SOURCE} ${G_TEXTURES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_TEXTURES_INCLUDE})
list(APPEND GAME_SOURCE ${G_TEXTURES_SOURCE})

# Skeleton Group
set(
    G_SKELETON_INCLUDE
	include/mu_skeletoninstance.h
	include/mu_skeletonmanager.h
)

set(
    G_SKELETON_SOURCE
	src/mu_skeletoninstance.cpp
	src/mu_skeletonmanager.cpp
)

source_group("Skeleton" FILES ${G_SKELETON_SOURCE} ${G_SKELETON_INCLUDE})
list(APPEND GAME_INCLUDE ${G_SKELETON_INCLUDE})
list(APPEND GAME_SOURCE ${G_SKELETON_SOURCE})

# States Group
set(
    G_STATES_INCLUDE
	include/mu_state.h
	include/mu_renderstate.h
	include/mu_controllerstate.h
)

set(
    G_STATES_SOURCE
	src/mu_state.cpp
	src/mu_renderstate.cpp
	src/mu_controllerstate.cpp
)

source_group("States" FILES ${G_STATES_SOURCE} ${G_STATES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_STATES_INCLUDE})
list(APPEND GAME_SOURCE ${G_STATES_SOURCE})

# Terrain Group
set(
    G_TERRAIN_INCLUDE
	include/mu_terrain.h
	include/t_terrain_consts.h
	include/t_terrain_cullingtree.h
	include/t_terrain_structs.h
)

set(
    G_TERRAIN_SOURCE
	src/mu_terrain.cpp
	src/t_terrain_cullingtree.cpp
)

source_group("Terrain" FILES ${G_TERRAIN_SOURCE} ${G_TERRAIN_INCLUDE})
list(APPEND GAME_INCLUDE ${G_TERRAIN_INCLUDE})
list(APPEND GAME_SOURCE ${G_TERRAIN_SOURCE})

# Items Group
set(
    G_ITEMS_INCLUDE
	include/res_item.h
	include/res_items.h
)

set(
    G_ITEMS_SOURCE
	src/res_items.cpp
)

source_group("Items" FILES ${G_ITEMS_SOURCE} ${G_ITEMS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ITEMS_INCLUDE})
list(APPEND GAME_SOURCE ${G_ITEMS_SOURCE})

# Math Group
set(
    G_MATH_INCLUDE
	include/mu_math_aabb.h
	include/mu_math_obb.h
	include/mu_math.h
)

set(
    G_MATH_SOURCE
	src/mu_math_aabb.cpp
	src/mu_math_obb.cpp
)

source_group("Math" FILES ${G_MATH_SOURCE} ${G_MATH_INCLUDE})
list(APPEND GAME_INCLUDE ${G_MATH_INCLUDE})
list(APPEND GAME_SOURCE ${G_MATH_SOURCE})

# Models Group
set(
    G_MODELS_INCLUDE
	include/mu_model_mesh.h
	include/mu_model_skeleton.h
	include/mu_model.h
	include/t_model_enums.h
)

set(
    G_MODELS_SOURCE
	src/mu_model.cpp
)

source_group("Models" FILES ${G_MODELS_SOURCE} ${G_MODELS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_MODELS_INCLUDE})
list(APPEND GAME_SOURCE ${G_MODELS_SOURCE})

# Scenes Group
set(
    G_SCENES_INCLUDE
	include/mu_scenemanager.h
)

set(
    G_SCENES_SOURCE
	src/mu_scenemanager.cpp
)

source_group("Scenes" FILES ${G_SCENES_SOURCE} ${G_SCENES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_SCENES_INCLUDE})
list(APPEND GAME_SOURCE ${G_SCENES_SOURCE})

# Scenes Base Group
set(
    G_SCENES_BASE_INCLUDE
	include/scn_base.h
)

source_group("Scenes\\Base" FILES ${G_SCENES_BASE_INCLUDE})
list(APPEND GAME_INCLUDE ${G_SCENES_BASE_INCLUDE})

# Scenes Intro Group
set(
    G_SCENES_INTRO_INCLUDE
	include/scn_intro.h
)

set(
    G_SCENES_INTRO_SOURCE
	src/scn_intro.cpp
)

source_group("Scenes\\Intro" FILES ${G_SCENES_INTRO_SOURCE} ${G_SCENES_INTRO_INCLUDE})
list(APPEND GAME_INCLUDE ${G_SCENES_INTRO_INCLUDE})
list(APPEND GAME_SOURCE ${G_SCENES_INTRO_SOURCE})

# Scenes Login Group
set(
    G_SCENES_LOGIN_INCLUDE
	include/scn_login.h
)

set(
    G_SCENES_LOGIN_SOURCE
	src/scn_login.cpp
)

source_group("Scenes\\Login" FILES ${G_SCENES_LOGIN_SOURCE} ${G_SCENES_LOGIN_INCLUDE})
list(APPEND GAME_INCLUDE ${G_SCENES_LOGIN_INCLUDE})
list(APPEND GAME_SOURCE ${G_SCENES_LOGIN_SOURCE})

# Scenes Characters Group
set(
    G_SCENES_CHARACTERS_INCLUDE
	include/scn_characters.h
)

set(
    G_SCENES_CHARACTERS_SOURCE
	src/scn_characters.cpp
)

source_group("Scenes\\Characters" FILES ${G_SCENES_CHARACTERS_SOURCE} ${G_SCENES_CHARACTERS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_SCENES_CHARACTERS_INCLUDE})
list(APPEND GAME_SOURCE ${G_SCENES_CHARACTERS_SOURCE})

# Scenes Game Group
set(
    G_SCENES_GAME_INCLUDE
	include/scn_game.h
)

set(
    G_SCENES_GAME_SOURCE
	src/scn_game.cpp
)

source_group("Scenes\\Game" FILES ${G_SCENES_GAME_SOURCE} ${G_SCENES_GAME_INCLUDE})
list(APPEND GAME_INCLUDE ${G_SCENES_GAME_INCLUDE})
list(APPEND GAME_SOURCE ${G_SCENES_GAME_SOURCE})

# Web Group
set(
    G_WEB_INCLUDE
	include/mu_webmanager.h
)

set(
    G_WEB_SOURCE
	src/mu_webmanager.cpp
)

source_group("Web" FILES ${G_WEB_SOURCE} ${G_WEB_INCLUDE})
list(APPEND GAME_INCLUDE ${G_WEB_INCLUDE})
list(APPEND GAME_SOURCE ${G_WEB_SOURCE})

# Web Requests Group
set(
    G_WEB_REQUESTS_INCLUDE
	include/web_requestbase.h
)

source_group("Web\\Requests" FILES ${G_WEB_REQUESTS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_WEB_REQUESTS_INCLUDE})

# Web Requests Group
set(
    G_WEB_REQUESTS_FILEDOWNLOAD_INCLUDE
	include/web_filedownload.h
)

set(
    G_WEB_REQUESTS_FILEDOWNLOAD_SOURCE
	src/web_filedownload.cpp
)

source_group("Web\\Requests\\File Download" FILES ${G_WEB_REQUESTS_FILEDOWNLOAD_SOURCE} ${G_WEB_REQUESTS_FILEDOWNLOAD_INCLUDE})
list(APPEND GAME_INCLUDE ${G_WEB_REQUESTS_FILEDOWNLOAD_INCLUDE})
list(APPEND GAME_SOURCE ${G_WEB_REQUESTS_FILEDOWNLOAD_SOURCE})

# Update Group
set(
    G_UPDATE_INCLUDE
	include/mu_updatemanager.h
)

set(
    G_UPDATE_SOURCE
	src/mu_updatemanager.cpp
)

source_group("Update" FILES ${G_UPDATE_SOURCE} ${G_UPDATE_INCLUDE})
list(APPEND GAME_INCLUDE ${G_UPDATE_INCLUDE})
list(APPEND GAME_SOURCE ${G_UPDATE_SOURCE})

# Update Steps Group
set(
    G_UPDATE_STEPS_INCLUDE
	include/upd_base.h
	include/upd_enum.h
)

source_group("Update\\Steps" FILES ${G_UPDATE_STEPS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_UPDATE_STEPS_INCLUDE})

# Update Steps Retrieve Servers Group
set(
    G_UPDATE_STEPS_RETRIEVESERVERS_INCLUDE
	include/upd_retrieveservers.h
)

set(
    G_UPDATE_STEPS_RETRIEVESERVERS_SOURCE
	src/upd_retrieveservers.cpp
)

source_group("Update\\Steps\\Retrieve Servers" FILES ${G_UPDATE_STEPS_RETRIEVESERVERS_SOURCE} ${G_UPDATE_STEPS_RETRIEVESERVERS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_UPDATE_STEPS_RETRIEVESERVERS_INCLUDE})
list(APPEND GAME_SOURCE ${G_UPDATE_STEPS_RETRIEVESERVERS_SOURCE})

# Environment Group
set(
    G_ENVIRONMENT_INCLUDE
	include/mu_environment.h
)

set(
    G_ENVIRONMENT_SOURCE
	src/mu_environment.cpp
	src/mu_environment_terrain.cpp
)

source_group("Environment" FILES ${G_ENVIRONMENT_SOURCE} ${G_ENVIRONMENT_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ENVIRONMENT_INCLUDE})
list(APPEND GAME_SOURCE ${G_ENVIRONMENT_SOURCE})

# Environment Group
set(
    G_ENVIRONMENT_OBJECTS_INCLUDE
	include/mu_environment_objects.h
	include/t_object_structs.h
)

set(
    G_ENVIRONMENT_OBJECTS_SOURCE
	src/mu_environment_objects.cpp
)

source_group("Environment\\Objects" FILES ${G_ENVIRONMENT_OBJECTS_SOURCE} ${G_ENVIRONMENT_OBJECTS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ENVIRONMENT_OBJECTS_INCLUDE})
list(APPEND GAME_SOURCE ${G_ENVIRONMENT_OBJECTS_SOURCE})

# Environment Characters Group
set(
    G_ENVIRONMENT_CHARACTERS_INCLUDE
	include/mu_environment_characters.h
	include/t_character_structs.h
)

set(
    G_ENVIRONMENT_CHARACTERS_SOURCE
	src/mu_environment_characters.cpp
	src/t_character_move.cpp
)

source_group("Environment\\Characters" FILES ${G_ENVIRONMENT_CHARACTERS_SOURCE} ${G_ENVIRONMENT_CHARACTERS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ENVIRONMENT_CHARACTERS_INCLUDE})
list(APPEND GAME_SOURCE ${G_ENVIRONMENT_CHARACTERS_SOURCE})

# Environment Controller Group
set(
    G_ENVIRONMENT_CONTROLLER_INCLUDE
	include/mu_environment_controller.h
)

set(
    G_ENVIRONMENT_CONTROLLER_SOURCE
	src/mu_environment_controller.cpp
)

source_group("Environment\\Controller" FILES ${G_ENVIRONMENT_CONTROLLER_SOURCE} ${G_ENVIRONMENT_CONTROLLER_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ENVIRONMENT_CONTROLLER_INCLUDE})
list(APPEND GAME_SOURCE ${G_ENVIRONMENT_CONTROLLER_SOURCE})

# Environment Entity Group
set(
    G_ENVIRONMENT_ENTITY_INCLUDE
	include/mu_entity_light.h
	include/mu_entity.h
)

set(
    G_ENVIRONMENT_ENTITY_SOURCE
	src/mu_entity.cpp
)

source_group("Environment\\Entity" FILES ${G_ENVIRONMENT_ENTITY_SOURCE} ${G_ENVIRONMENT_ENTITY_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ENVIRONMENT_ENTITY_INCLUDE})
list(APPEND GAME_SOURCE ${G_ENVIRONMENT_ENTITY_SOURCE})

# Environment Joints Group
set(
    G_ENVIRONMENT_JOINTS_INCLUDE
	include/mu_environment_joints.h
)

set(
    G_ENVIRONMENT_JOINTS_SOURCE
	src/mu_environment_joints.cpp
)

source_group("Environment\\Joints" FILES ${G_ENVIRONMENT_JOINTS_SOURCE} ${G_ENVIRONMENT_JOINTS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ENVIRONMENT_JOINTS_INCLUDE})
list(APPEND GAME_SOURCE ${G_ENVIRONMENT_JOINTS_SOURCE})

# Joints Group
set(
    G_JOINTS_INCLUDE
	include/t_joint_base.h
	include/t_joint_config.h
	include/t_joint_create.h
	include/t_joint_enum.h
	include/t_joint_render.h
	include/t_joint_tails.h
)

set(
    G_JOINTS_SOURCE
	src/t_joint_base.cpp
)

source_group("Joints" FILES ${G_JOINTS_SOURCE} ${G_JOINTS_INCLUDE})
list(APPEND GAME_INCLUDE ${G_JOINTS_INCLUDE})
list(APPEND GAME_SOURCE ${G_JOINTS_SOURCE})

# Joints Entity Group
set(
    G_JOINTS_ENTITY_INCLUDE
	include/t_joint_entity.h
)

source_group("Joints\\Entity" FILES ${G_JOINTS_ENTITY_INCLUDE})
list(APPEND GAME_INCLUDE ${G_JOINTS_ENTITY_INCLUDE})

# Joints Types Group
# Thunder01 v7
set(
    G_JOINTS_THUNDER01_V7
	src/t_joint_thunder01_v7.cpp
	include/t_joint_thunder01_v7.h
)

source_group("Joints\\Types\\Thunder01\\V7" FILES ${G_JOINTS_THUNDER01_V7})
list(APPEND JOINTS_SOURCE ${G_JOINTS_THUNDER01_V7})

# Environment Particles Group
set(
    G_ENVIRONMENT_PARTICLES_INCLUDE
	include/mu_environment_particles.h
)

set(
    G_ENVIRONMENT_PARTICLES_SOURCE
	src/mu_environment_particles.cpp
)

source_group("Environment\\Particles" FILES ${G_ENVIRONMENT_PARTICLES_SOURCE} ${G_ENVIRONMENT_PARTICLES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_ENVIRONMENT_PARTICLES_INCLUDE})
list(APPEND GAME_SOURCE ${G_ENVIRONMENT_PARTICLES_SOURCE})

# Particles Group
set(
    G_PARTICLES_INCLUDE
	include/t_particle_base.h
	include/t_particle_config.h
	include/t_particle_create.h
	include/t_particle_enum.h
	include/t_particle_macros.h
	include/t_particle_render.h
)

set(
    G_PARTICLES_SOURCE
	src/t_particle_base.cpp
)

source_group("Particles" FILES ${G_PARTICLES_SOURCE} ${G_PARTICLES_INCLUDE})
list(APPEND GAME_INCLUDE ${G_PARTICLES_INCLUDE})
list(APPEND GAME_SOURCE ${G_PARTICLES_SOURCE})

# Particles Entity Group
set(
    G_PARTICLES_ENTITY_INCLUDE
	include/t_particle_entity.h
)

source_group("Particles\\Entity" FILES ${G_PARTICLES_ENTITY_INCLUDE})
list(APPEND GAME_INCLUDE ${G_PARTICLES_ENTITY_INCLUDE})

# Particles Types Group
# BlueBlur v0
set(
    G_PARTICLES_BLUEBLUR_V0
	src/t_particle_blueblur_v0.cpp
	include/t_particle_blueblur_v0.h
)
source_group("Particles\\Types\\BlueBlur\\V0" FILES ${G_PARTICLES_BLUEBLUR_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_BLUEBLUR_V0})
# BlueBlur v1
set(
    G_PARTICLES_BLUEBLUR_V1
	src/t_particle_blueblur_v1.cpp
	include/t_particle_blueblur_v1.h
)
source_group("Particles\\Types\\BlueBlur\\V1" FILES ${G_PARTICLES_BLUEBLUR_V1})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_BLUEBLUR_V1})
# Bubble v0
set(
    G_PARTICLES_BUBBLE_V0
	src/t_particle_bubble_v0.cpp
	include/t_particle_bubble_v0.h
)
source_group("Particles\\Types\\Bubble\\V0" FILES ${G_PARTICLES_BUBBLE_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_BUBBLE_V0})
# Effect v0
set(
    G_PARTICLES_EFFECT_V0
	src/t_particle_effect_v0.cpp
	include/t_particle_effect_v0.h
)
source_group("Particles\\Types\\Effect\\V0" FILES ${G_PARTICLES_EFFECT_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V0})
# Effect v1
set(
    G_PARTICLES_EFFECT_V1
	src/t_particle_effect_v1.cpp
	include/t_particle_effect_v1.h
)
source_group("Particles\\Types\\Effect\\V1" FILES ${G_PARTICLES_EFFECT_V1})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V1})
# Effect v2
set(
    G_PARTICLES_EFFECT_V2
	src/t_particle_effect_v2.cpp
	include/t_particle_effect_v2.h
)
source_group("Particles\\Types\\Effect\\V2" FILES ${G_PARTICLES_EFFECT_V2})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V2})
# Effect v3
set(
    G_PARTICLES_EFFECT_V3
	src/t_particle_effect_v3.cpp
	include/t_particle_effect_v3.h
)
source_group("Particles\\Types\\Effect\\V3" FILES ${G_PARTICLES_EFFECT_V3})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V3})
# Effect v4
set(
    G_PARTICLES_EFFECT_V4
	src/t_particle_effect_v4.cpp
	include/t_particle_effect_v4.h
)
source_group("Particles\\Types\\Effect\\V4" FILES ${G_PARTICLES_EFFECT_V4})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V4})
# Effect v5
set(
    G_PARTICLES_EFFECT_V5
	src/t_particle_effect_v5.cpp
	include/t_particle_effect_v5.h
)
source_group("Particles\\Types\\Effect\\V5" FILES ${G_PARTICLES_EFFECT_V5})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V5})
# Effect v6
set(
    G_PARTICLES_EFFECT_V6
	src/t_particle_effect_v6.cpp
	include/t_particle_effect_v6.h
)
source_group("Particles\\Types\\Effect\\V6" FILES ${G_PARTICLES_EFFECT_V6})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V6})
# Effect v7
set(
    G_PARTICLES_EFFECT_V7
	src/t_particle_effect_v7.cpp
	include/t_particle_effect_v7.h
)
source_group("Particles\\Types\\Effect\\V7" FILES ${G_PARTICLES_EFFECT_V7})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_EFFECT_V7})
# Flare02 v0
set(
    G_PARTICLES_FLARE02_V0
	src/t_particle_flare02_v0.cpp
	include/t_particle_flare02_v0.h
)
source_group("Particles\\Types\\Flare02\\V0" FILES ${G_PARTICLES_FLARE02_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLARE02_V0})
# FlareBlue v0
set(
    G_PARTICLES_FLAREBLUE_V0
	src/t_particle_flareblue_v0.cpp
	include/t_particle_flareblue_v0.h
)
source_group("Particles\\Types\\FlareBlue\\V0" FILES ${G_PARTICLES_FLAREBLUE_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLAREBLUE_V0})
# FlareBlue v1
set(
    G_PARTICLES_FLAREBLUE_V1
	src/t_particle_flareblue_v1.cpp
	include/t_particle_flareblue_v1.h
)
source_group("Particles\\Types\\FlareBlue\\V1" FILES ${G_PARTICLES_FLAREBLUE_V1})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLAREBLUE_V1})
# Flower01 v0
set(
    G_PARTICLES_FLOWER01_V0
	src/t_particle_flower01_v0.cpp
	include/t_particle_flower01_v0.h
)
source_group("Particles\\Types\\Flower01\\V0" FILES ${G_PARTICLES_FLOWER01_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLOWER01_V0})
# Flower01 v1
set(
    G_PARTICLES_FLOWER01_V1
	src/t_particle_flower01_v1.cpp
	include/t_particle_flower01_v1.h
)
source_group("Particles\\Types\\Flower01\\V1" FILES ${G_PARTICLES_FLOWER01_V1})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLOWER01_V1})
# Flower02 v0
set(
    G_PARTICLES_FLOWER02_V0
	src/t_particle_flower02_v0.cpp
	include/t_particle_flower02_v0.h
)
source_group("Particles\\Types\\Flower02\\V0" FILES ${G_PARTICLES_FLOWER02_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLOWER02_V0})
# Flower02 v1
set(
    G_PARTICLES_FLOWER02_V1
	src/t_particle_flower02_v1.cpp
	include/t_particle_flower02_v1.h
)
source_group("Particles\\Types\\Flower02\\V1" FILES ${G_PARTICLES_FLOWER02_V1})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLOWER02_V1})
# Flower03 v0
set(
    G_PARTICLES_FLOWER03_V0
	src/t_particle_flower03_v0.cpp
	include/t_particle_flower03_v0.h
)
source_group("Particles\\Types\\Flower03\\V0" FILES ${G_PARTICLES_FLOWER03_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLOWER03_V0})
# Flower03 v1
set(
    G_PARTICLES_FLOWER03_V1
	src/t_particle_flower03_v1.cpp
	include/t_particle_flower03_v1.h
)
source_group("Particles\\Types\\Flower03\\V1" FILES ${G_PARTICLES_FLOWER03_V1})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_FLOWER03_V1})
# Smoke01 v0
set(
    G_PARTICLES_SMOKE01_V0
	src/t_particle_smoke01_v0.cpp
	include/t_particle_smoke01_v0.h
)
source_group("Particles\\Types\\Smoke01\\V0" FILES ${G_PARTICLES_SMOKE01_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_SMOKE01_V0})
# Smoke05 v0
set(
    G_PARTICLES_SMOKE05_V0
	src/t_particle_smoke05_v0.cpp
	include/t_particle_smoke05_v0.h
)
source_group("Particles\\Types\\Smoke05\\V0" FILES ${G_PARTICLES_SMOKE05_V0})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_SMOKE05_V0})
# Smoke05 v1
set(
    G_PARTICLES_SMOKE05_V1
	src/t_particle_smoke05_v1.cpp
	include/t_particle_smoke05_v1.h
)
source_group("Particles\\Types\\Smoke05\\V1" FILES ${G_PARTICLES_SMOKE05_V1})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_SMOKE05_V1})
# TrueFire Red v5
set(
    G_PARTICLES_TRUEFIRE_RED_V5
	src/t_particle_truefire_red_v5.cpp
	include/t_particle_truefire_red_v5.h
)
source_group("Particles\\Types\\TrueFire_Red\\V5" FILES ${G_PARTICLES_TRUEFIRE_RED_V5})
list(APPEND PARTICLES_SOURCE ${G_PARTICLES_TRUEFIRE_RED_V5})