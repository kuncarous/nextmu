cmake_minimum_required (VERSION 3.28)

project(Shared)

set(
    RESOURCE
	mu_main.rc
    icon.ico
    small.ico
)

set(
    INCLUDE
    stdafx.h
    targetver.h
    resource.h
)

set(
    SOURCE
    stdafx.cpp
    mu_main.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DWINVER=0x0601 -D_WIN32_WINNT=0x0601)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_executable(NextMU WIN32 ${SOURCE} ${INCLUDE} ${RESOURCE})
target_link_libraries(NextMU PUBLIC Game)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(
        NextMU
        PRIVATE
            d3d11.lib
            d3d12.lib
            ninput.lib
            winmm.lib
            dxgi.lib
            shcore.lib
            dxguid.lib
    )
endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("resources" FILES ${RESOURCE})