cmake_minimum_required (VERSION 3.28)

project(Shared)

set(
    INCLUDE
	include/shared_binaryreader.h
	include/shared_enums.h
	include/shared_json.h
	include/shared_operatingsystem_android.h
	include/shared_operatingsystem_backend.h
	include/shared_operatingsystem_disable.h
	include/shared_operatingsystem_io.h
	include/shared_operatingsystem.h
	include/shared_precompiled.h
	include/shared_standardtypes.h
)

set(
    SOURCE
    src/shared_string_encoding.cpp
    src/shared_operatingsystem_windows.cpp
    src/shared_operatingsystem_android.cpp
    src/shared_operatingsystem_backend.cpp
    src/shared_operatingsystem_io.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DWINVER=0x0601 -D_WIN32_WINNT=0x0601)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_library(Shared STATIC ${SOURCE} ${INCLUDE})
target_link_libraries(
    Shared
    PUBLIC
        Boost::algorithm
        Boost::serialization
        fmt
        SDL2
        nlohmann_json
)
target_precompile_headers(Shared PRIVATE include/shared_precompiled.h)
target_include_directories(Shared PUBLIC include)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})