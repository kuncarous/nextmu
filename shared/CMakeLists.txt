cmake_minimum_required (VERSION 3.22)

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
if (PLATFORM_MACOS)
    set(
        SOURCE
        ${SOURCE}
        src/shared_operatingsystem_macos.mm
    )
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_library(Shared STATIC ${SOURCE} ${INCLUDE})

target_compile_definitions(Shared PUBLIC -DNEXTMU_COMPANY_NAME="${NEXTMU_COMPANY_NAME}" -DNEXTMU_GAME_NAME="${NEXTMU_GAME_NAME}")

if (PLATFORM_WIN32)
    target_compile_definitions(Shared PUBLIC -DWINVER=0x0601 -D_WIN32_WINNT=0x0601)
    set(MSVC_RELEASE_COMPILE_OPTIONS /Oi /GS-)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MSVC_RELEASE_COMPILE_OPTIONS ${MSVC_RELEASE_COMPILE_OPTIONS} /arch:SSE2)
    endif()
    target_compile_options(Shared PUBLIC "$<$<CONFIG:RELEASE>:${MSVC_RELEASE_COMPILE_OPTIONS}>")
elseif (PLATFORM_MACOS)
    target_compile_definitions(Shared PUBLIC -DMACOS=1)
elseif (PLATFORM_ANDROID) 
    target_compile_definitions(Shared PUBLIC -DANDROID=1)
endif()

target_link_libraries(
    Shared
    PUBLIC
        Boost::algorithm
        Boost::serialization
        fmt
        SDL2
        nlohmann_json
)
target_precompile_headers(Shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/shared_precompiled.h)
target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})