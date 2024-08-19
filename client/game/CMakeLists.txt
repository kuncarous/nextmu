cmake_minimum_required (VERSION 3.22)

project(Game)
include(SourceTree.cmake)

get_supported_backends(ENGINE_LIBRARIES)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_library(Game STATIC ${GAME_SOURCE} ${GAME_INCLUDE} ${JOINTS_SOURCE} ${PARTICLES_SOURCE})
target_compile_definitions(
    Game
    PUBLIC
        -DAS_USE_NAMESPACE=1
        -DNEXTMU_OPENID_URL="${NEXTMU_OPENID_URL}"
        -DNEXTMU_OPENID_CLIENT_ID="${NEXTMU_OPENID_CLIENT_ID}"
)

target_link_libraries(
    Game
    PUBLIC
        Shared
        zlibstatic
        FreeImage
        glm
        EnTT
        cryptopp
        DiligentFX
        Diligent-TargetPlatform
        Diligent-TextureLoader
        Diligent-Common
        Diligent-GraphicsTools
        ${ENGINE_LIBRARIES}
        jwt-cpp::jwt-cpp
        CURL::libcurl
        libcef_dll_wrapper
)

list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include ${CEF_INCLUDE_PATH})
list(APPEND GAME_PRIVATE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/Detour/Include)

if(UI_LIBRARY STREQUAL "NoesisGUI")
    list(APPEND GAME_PUBLIC_INCLUDES ${NEXTMU_CLIENT_BASE_DIR}/../dependencies/installed/noesisgui/Src/Packages/App/Interactivity/Include)
    list(APPEND GAME_PUBLIC_INCLUDES ${NEXTMU_CLIENT_BASE_DIR}/../dependencies/installed/noesisgui/Src/Packages/App/MediaElement/Include)
    list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/noesis/controls)
    list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/noesis/controls)
    list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/noesis/delegatecommand)
    list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/noesis/richtext)
    list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/noesis/localization)
endif()

target_link_libraries(
    Game
    PUBLIC
        angelscript
)

if(UI_LIBRARY STREQUAL "NoesisGUI")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../NoesisGUI.cmake)
    target_link_libraries(
        Game
        PUBLIC
            Noesis
    )
    target_link_directories(Game PUBLIC ${NOESISGUI_LIBRARY_DIR})
    list(APPEND GAME_PUBLIC_INCLUDES ${NOESISGUI_INCLUDE_DIR})
endif()

if(PLATFORM_WIN32 OR PLATFORM_LINUX OR PLATFORM_MACOS)
    target_link_libraries(
        Game
        PUBLIC
            Poco::Foundation
            Poco::Net
    )
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../SteamAudio.cmake)
target_link_libraries(
    Game
    PUBLIC
        phonon
)
target_link_directories(Game PUBLIC ${STEAMAUDIO_LIBRARY_DIR})
list(APPEND GAME_PUBLIC_INCLUDES ${STEAMAUDIO_INCLUDE_DIR})

target_precompile_headers(Game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/mu_precompiled.h)
target_include_directories(Game PUBLIC ${GAME_PUBLIC_INCLUDES} PRIVATE ${GAME_PRIVATE_INCLUDES})
set_source_files_properties(${GAME_SOURCE_NONPCH} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)