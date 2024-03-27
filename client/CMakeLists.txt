cmake_minimum_required (VERSION 3.22)

set(NEXTMU_COMPANY_NAME     "NextGames" CACHE STRING "Company Name")
set(NEXTMU_GAME_NAME        "NextMU" CACHE STRING "Game Name")

set(NEXTMU_VERSION_MAJOR    "0" CACHE STRING "Version Major")
set(NEXTMU_VERSION_MINOR	"0" CACHE STRING "Version Minor")
set(NEXTMU_VERSION_REVISION	"1" CACHE STRING "Version Revision")
set(NEXTMU_VERSION_SERIAL	"UUb335yJPFn7MFfdx6HtTBLrfuAxcDD49nNMQgXZFPbcdHF3sSXrBHqg5stc66bu" CACHE STRING "Version Serial")

set(NEXTMU_OPENID_URL "http://localhost:8080/" CACHE STRING "OpenID Connect Base URL")
set(NEXTMU_OPENID_CLIENT_ID "111111111111111@nextmu" CACHE STRING "Game Project Client ID")

set(NEXTMU_CLIENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_ANDROID_STL_TYPE c++_static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(
    NextMU
    VERSION ${NEXTMU_VERSION_MAJOR}.${NEXTMU_VERSION_MINOR}.${NEXTMU_VERSION_REVISION}
)

include(../SetPlatform.cmake)

if(PLATFORM_MACOS OR PLATFORM_IOS)
    set(BUILD_SHARED_LIBS OFF)
else()
    set(BUILD_SHARED_LIBS ON)
endif()

include(Dependencies.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(IS_CLIENT_SHARED "1")

add_subdirectory(../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)
add_subdirectory(game)

if (PLATFORM_MACOS)
    add_subdirectory(macos)
elseif (PLATFORM_WIN32)
    add_subdirectory(browser/windows)
    add_subdirectory(windows)
elseif (PLATFORM_LINUX)
    add_subdirectory(browser/linux)
    add_subdirectory(linux)
elseif (PLATFORM_ANDROID)
    add_subdirectory(android/lib)
endif()