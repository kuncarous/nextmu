cmake_minimum_required (VERSION 3.28)

project(
    NextMU
    VERSION 0.0.1
)

include(CMakeLists-Dependencies.txt)

add_subdirectory(shared)
add_subdirectory(client/game)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(client/windows)
endif()