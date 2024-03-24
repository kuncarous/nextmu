cmake_minimum_required (VERSION 3.22)

project(Shared)

set(
    INCLUDE
	include/shared_binaryreader.h
	include/shared_objectid.h
	include/shared_uuid.h
	include/shared_enums.h
	include/shared_json.h
	include/shared_crc32.h
	include/shared_memorybuffer.h
	include/shared_compression.h
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

if (IS_CLIENT_SHARED GREATER_EQUAL 1)
    target_compile_definitions(
        Shared
        PUBLIC
            -DNEXTMU_CLIENT_SHARED=1
            -DNEXTMU_COMPANY_NAME="${NEXTMU_COMPANY_NAME}"
            -DNEXTMU_GAME_NAME="${NEXTMU_GAME_NAME}"
            -DNEXTMU_VERSION_MAJOR=${NEXTMU_VERSION_MAJOR}
            -DNEXTMU_VERSION_MINOR=${NEXTMU_VERSION_MINOR}
            -DNEXTMU_VERSION_REVISION=${NEXTMU_VERSION_REVISION}
            -DNEXTMU_VERSION_SERIAL="${NEXTMU_VERSION_SERIAL}"
    )

    if (UI_LIBRARY STREQUAL "NoesisGUI")
        target_compile_definitions(Shared PUBLIC -DNEXTMU_USE_NOESISGUI=1)
    endif()
endif()

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
        Boost::headers
        Boost::filesystem
        Boost::serialization
        Boost::uuid
        Boost::regex
        fmt::fmt
        nlohmann_json::nlohmann_json
        fifo_map::fifo_map
        ZLIB::ZLIB
)
if(IS_CLIENT_SHARED GREATER_EQUAL 1)
    target_link_libraries(
        Shared
        PUBLIC
            SDL2::SDL2
    )
else()
    target_link_libraries(
        Shared
        PUBLIC
            Qt6::Core
    )
endif()
target_precompile_headers(Shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/shared_precompiled.h)
target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})