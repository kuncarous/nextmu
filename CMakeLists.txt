cmake_minimum_required (VERSION 3.22)

project(
    NextMU
    VERSION 0.0.1
)

set(NEXTMU_COMPANY_NAME "NextGames" CACHE STRING "Company Name")
set(NEXTMU_GAME_NAME "NextMU" CACHE STRING "Game Name")

include(CMakeLists-SetPlatform.txt)
include(CMakeLists-Dependencies.txt)

add_subdirectory(shared)
add_subdirectory(client/game)

if (PLATFORM_MACOS)
    add_subdirectory(client/macos)
endif()
if (PLATFORM_WIN32)
    add_subdirectory(client/windows)
endif()
if (PLATFORM_ANDROID)
    add_subdirectory(client/android/lib)
endif()