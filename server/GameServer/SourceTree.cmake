# Qt Group
set(
    G_QT_ROOT_INCLUDE
	include/mu_precompiled.h
)

set(
    G_QT_ROOT_SOURCE
	src/app_environment.h
	src/import_qml_components_plugins.h
	src/import_qml_plugins.h
	src/main.cpp
)

source_group("Qt" FILES ${G_QT_ROOT_SOURCE} ${G_QT_ROOT_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_QT_ROOT_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_QT_ROOT_SOURCE})

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
list(APPEND GAMESERVER_INCLUDE ${G_ROOT_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_ROOT_SOURCE})

# Root Application Group
set(
    G_ROOT_APPLICATION_INCLUDE
	include/n_root_application.h
)

set(
    G_ROOT_APPLICATION_SOURCE
	src/n_root_application.cpp
)

source_group("Root\\Application" FILES ${G_ROOT_APPLICATION_SOURCE} ${G_ROOT_APPLICATION_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_ROOT_APPLICATION_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_ROOT_APPLICATION_SOURCE})

# Root Events Group
set(
    G_ROOT_EVENTS_SOURCE
	src/n_root_events.cpp
)

source_group("Root\\Events" FILES ${G_ROOT_EVENTS_SOURCE})
list(APPEND GAMESERVER_SOURCE ${G_ROOT_EVENTS_SOURCE})

# Root Thread Group
set(
    G_ROOT_THREAD_INCLUDE
	include/n_root_thread.h
)

set(
    G_ROOT_THREAD_SOURCE
	src/n_root_thread.cpp
)

source_group("Root\\Thread" FILES ${G_ROOT_THREAD_SOURCE} ${G_ROOT_THREAD_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_ROOT_THREAD_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_ROOT_THREAD_SOURCE})

# Detour
set(
    G_DETOUR_INCLUDE
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
    G_DETOUR_SOURCE
    Detour/Source/DetourAlloc.cpp
    Detour/Source/DetourAssert.cpp
    Detour/Source/DetourCommon.cpp
    Detour/Source/DetourNavMesh.cpp
    Detour/Source/DetourNavMeshBuilder.cpp
    Detour/Source/DetourNavMeshQuery.cpp
    Detour/Source/DetourNode.cpp
)

source_group("Detour" FILES ${G_DETOUR_SOURCE} ${G_DETOUR_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_DETOUR_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_DETOUR_SOURCE})

# Directories Group
set(
    G_DIRECTORIES_INCLUDE
	include/mu_directories.h
)

set(
    G_DIRECTORIES_SOURCE
	src/mu_directories.cpp
)

source_group("Directories" FILES ${G_DIRECTORIES_SOURCE} ${G_DIRECTORIES_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_DIRECTORIES_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_DIRECTORIES_SOURCE})

# Logger Group
set(
    G_LOGGER_INCLUDE
	include/mu_logger.h
	include/n_logger_message.h
	include/n_logger_console.h
	include/n_logger_file.h
)

set(
    G_LOGGER_SOURCE
	src/mu_logger.cpp
	src/n_logger_message.cpp
	src/n_logger_console.cpp
	src/n_logger_file.cpp
)

source_group("Logger" FILES ${G_LOGGER_SOURCE} ${G_LOGGER_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_LOGGER_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_LOGGER_SOURCE})

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
list(APPEND GAMESERVER_INCLUDE ${G_TIMER_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_TIMER_SOURCE})

# UI Events Group
set(
    G_UI_EVENTS_INCLUDE
	include/mu_uievents.h
	include/n_uievent_base.h
)

source_group("UI\\Events" FILES ${G_UI_EVENTS_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_UI_EVENTS_INCLUDE})

# UI Events FPS Group
set(
    G_UIEVENT_FPS_INCLUDE
	include/n_uievent_updatefps.h
)

source_group("UI\\Events\\FPS" FILES ${G_UIEVENT_FPS_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_UIEVENT_FPS_INCLUDE})

# UI Events Console Group
set(
    G_UIEVENT_CONSOLE_INCLUDE
	include/n_uievent_updateconsole.h
)

source_group("UI\\Events\\Console" FILES ${G_UIEVENT_CONSOLE_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_UIEVENT_CONSOLE_INCLUDE})

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
list(APPEND GAMESERVER_INCLUDE ${G_THREADING_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_THREADING_SOURCE})

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
list(APPEND GAMESERVER_INCLUDE ${G_MATH_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_MATH_SOURCE})

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
list(APPEND GAMESERVER_INCLUDE ${G_MODELS_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_MODELS_SOURCE})

# Resources Group
set(
    G_RESOURCES_INCLUDE
	include/mu_resourcesmanager.h
)

set(
    G_RESOURCES_SOURCE
	src/mu_resourcesmanager.cpp
)

source_group("Resources" FILES ${G_RESOURCES_SOURCE} ${G_RESOURCES_INCLUDE})
list(APPEND GAMESERVER_INCLUDE ${G_RESOURCES_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_RESOURCES_SOURCE})

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
list(APPEND GAMESERVER_INCLUDE ${G_UTILS_INCLUDE})
list(APPEND GAMESERVER_SOURCE ${G_UTILS_SOURCE})