cmake_minimum_required (VERSION 3.22)

set(NEXTMU_COMPANY_NAME     "NextGames" CACHE STRING "Company Name")
set(NEXTMU_GAME_NAME        "NextMU" CACHE STRING "Game Name")
set(NEXTMU_VERSION_MAJOR    "0" CACHE STRING "Version Major")
set(NEXTMU_VERSION_MINOR	"0" CACHE STRING "Version Minor")
set(NEXTMU_VERSION_REVISION	"1" CACHE STRING "Version Revision")
set(NEXTMU_VERSION_SERIAL	"UUb335yJPFn7MFfdx6HtTBLrfuAxcDD49nNMQgXZFPbcdHF3sSXrBHqg5stc66bu" CACHE STRING "Version Serial")

project(
    NextMU
    VERSION ${NEXTMU_VERSION_MAJOR}.${NEXTMU_VERSION_MINOR}.${NEXTMU_VERSION_REVISION}
)

include(../SetPlatform.cmake)
include(Dependencies.cmake)

set(IS_CLIENT_SHARED "1")
add_subdirectory(../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)
add_subdirectory(game)

if (PLATFORM_MACOS)
    add_subdirectory(macos)
endif()
if (PLATFORM_WIN32)
    add_subdirectory(windows)
endif()
if (PLATFORM_ANDROID)
    add_subdirectory(android/lib)
endif()