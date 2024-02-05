include(FetchContent)

set(UI_LIBRARY "NoesisGUI") # Values : NoesisGUI, RmlUI
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(BOOST_INCLUDE_LIBRARIES algorithm serialization CACHE STRING "" FORCE)
set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.84.0
)
FetchContent_MakeAvailable(Boost)

set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
)
FetchContent_MakeAvailable(fmt)

if (PLATFORM_IOS OR PLATFORM_MACOS)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
else()
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
endif()
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG 15ead9a40d09a1eb9972215cceac2bf29c9b77f6
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG 9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG fedcb920ce0068c35ffbc66fd4e84864e6ef71ef
)
FetchContent_MakeAvailable(entt)

set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CRYPTOPP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    cryptopp-cmake
    GIT_REPOSITORY https://github.com/abdes/cryptopp-cmake
    GIT_TAG f815f6284684be6ab03af4b6c273359331c61241
)
FetchContent_MakeAvailable(cryptopp-cmake)

FetchContent_Declare(
    ZLIB
    GIT_REPOSITORY https://github.com/madler/zlib
    GIT_TAG 09155eaa2f9270dc4ed1fa13e2b4b2613e6e4851
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(ZLIB)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(zlibstatic PRIVATE -Wno-shorten-64-to-32)
endif()
add_library(ZLIB::ZLIB ALIAS zlibstatic)
list(APPEND THIRD_PARTY_TARGETS zlibstatic)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/ZLIB-extra.cmake
[=[
  get_target_property(ZLIB_INCLUDE_DIRS zlibstatic INCLUDE_DIRECTORIES)
  include_directories("${ZLIB_INCLUDE_DIRS}")
  set(ZLIB_LIBRARIES zlibstatic)
]=])

set(PNG_BUILD_ZLIB OFF CACHE BOOL "Disable find_package(zlib) to find ZLib location" FORCE)
set(PNG_SHARED OFF CACHE BOOL "Build shared version of libpng" FORCE)
set(PNG_EXECUTABLES OFF CACHE BOOL "Build libpng executables" FORCE)
set(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)
set(SKIP_INSTALL_ALL ON)
if(PLATFORM_TVOS)
    # libpng does not support tvOS, but it does support iOS, so we can use the iOS target
    set(IOS TRUE)
endif()
FetchContent_Declare(
    PNG
    GIT_REPOSITORY https://git.code.sf.net/p/libpng/code
    GIT_TAG libpng16
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(PNG)
set_target_properties(png_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
get_target_property(PNG_INCLUDE_DIRS png_static INCLUDE_DIRECTORIES)
target_include_directories(png_static PUBLIC ${PNG_INCLUDE_DIRS})
add_library(PNG::PNG ALIAS png_static)
list(APPEND THIRD_PARTY_TARGETS png_static)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/PNG-extra.cmake
[=[
  get_target_property(PNG_INCLUDE_DIRS PNG::PNG INCLUDE_DIRECTORIES)
  include_directories("${PNG_INCLUDE_DIRS}")
  set(PNG_LIBRARIES PNG::PNG)
]=])

if (UI_LIBRARY STREQUAL "RmlUI")
    FetchContent_Declare(
        BZip2
        GIT_REPOSITORY https://gitlab.com/bzip2/bzip2
        GIT_TAG 6a8690fc8d26c815e798c588f796eabe9d684cf0
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(BZip2)

    FetchContent_Declare(
        BrotliDec
        GIT_REPOSITORY https://github.com/google/brotli
        GIT_TAG ed738e842d2fbdf2d6459e39267a633c4a9b2f5d
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(BrotliDec)
    add_library(BrotliDec::brotlidec ALIAS brotlidec)
    file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/BrotliDec-extra.cmake
    [=[
    get_target_property(BROTLIDEC_INCLUDE_DIRS BrotliDec::brotlidec INCLUDE_DIRECTORIES)
    set(BROTLIDEC_LIBRARIES BrotliDec::brotlidec)
    ]=])

    FetchContent_Declare(
        Freetype
        GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
        GIT_TAG 920c5502cc3ddda88f6c7d85ee834ac611bb11cc
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(Freetype)
    add_library(Freetype::freetype ALIAS freetype)
    file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/Freetype-extra.cmake
    [=[
    get_target_property(FREETYPE_INCLUDE_DIRS Freetype::freetype INCLUDE_DIRECTORIES)
    set(FREETYPE_LIBRARIES Freetype::freetype)
    ]=])

    FetchContent_Declare(
        RmlUi
        GIT_REPOSITORY https://github.com/mikke89/RmlUi
        GIT_TAG b8f8e2e1237a967ad5b2d2eae9a751aace428a67
    )
    FetchContent_MakeAvailable(RmlUi)
endif()

set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_INSTALL_CORE OFF CACHE BOOL "" FORCE)
set(DILIGENT_INSTALL_FX OFF CACHE BOOL "" FORCE)
set(DILIGENT_INSTALL_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_INSTALL_TOOLS OFF CACHE BOOL "" FORCE)
set(DILIGENT_MSVC_RELEASE_COMPILE_OPTIONS "/arch:SSE2" CACHE STRING "" FORCE)
FetchContent_Declare(
    DiligentEngine
    GIT_REPOSITORY https://github.com/DiligentGraphics/DiligentEngine
    GIT_TAG 7d46b75850ca3fe1f7778775ea0fedb28146327e
)
FetchContent_MakeAvailable(DiligentEngine)

add_subdirectory(../dependencies/freeimage ${CMAKE_CURRENT_BINARY_DIR}/dependencies/freeimage)