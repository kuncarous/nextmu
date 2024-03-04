cmake_minimum_required (VERSION 3.22)

project(Game)
include(SourceTree.cmake)

get_supported_backends(ENGINE_LIBRARIES)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_library(Game STATIC ${GAME_SOURCE} ${GAME_INCLUDE} ${JOINTS_SOURCE} ${PARTICLES_SOURCE})

if(UI_LIBRARY STREQUAL "RmlUI")
    target_compile_definitions(
        Shared
        PUBLIC
            -DRMLUI_STATIC_LIB=1
    )
endif()

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
)

list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND GAME_PRIVATE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/Detour/Include)

if(UI_LIBRARY STREQUAL "NoesisGUI")
    list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/noesis/richtext)
    list(APPEND GAME_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/noesis/localization)
endif()

if(UI_LIBRARY STREQUAL "RmlUI")
    target_link_libraries(
        Game
        PUBLIC
            RmlCore
            debug RmlDebugger
    )
elseif(UI_LIBRARY STREQUAL "NoesisGUI")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../NoesisGUI.cmake)
    target_link_libraries(
        Game
        PUBLIC
            Noesis
    )
    target_link_directories(Game PUBLIC ${NOESISGUI_LIBRARY_DIR})
    list(APPEND GAME_PUBLIC_INCLUDES ${NOESISGUI_INCLUDE_DIR})
endif()

target_precompile_headers(Game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/mu_precompiled.h)
target_include_directories(Game PUBLIC ${GAME_PUBLIC_INCLUDES} PRIVATE ${GAME_PRIVATE_INCLUDES})