# set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/../.cpm-cache")
include(../CPM.cmake)
include(../SetPlatform.cmake)

set(BUILD_TESTING_BACKUP ${BUILD_TESTING})
set(BUILD_TESTING OFF)

if (NOT BUILD_SHARED_LIBS)
    set(BUILD_STATIC_LIBS ON)
    set(OPENSSL_USE_STATIC_LIBS ON)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(UI_LIBRARY "NoesisGUI")
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(-Wno-unsafe-buffer-usage)
    link_libraries(-Wl,--undefined-version)
endif()

CPMAddPackage(
    NAME ZLIB
    GIT_TAG 09155eaa2f9270dc4ed1fa13e2b4b2613e6e4851
    GITHUB_REPOSITORY madler/zlib
    OPTIONS
        "CMAKE_POSITION_INDEPENDENT_CODE TRUE"
)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(zlibstatic PRIVATE -Wno-shorten-64-to-32)
endif()
get_target_property(ZLIB_INCLUDE_DIRS zlibstatic INCLUDE_DIRECTORIES)
target_include_directories(zlibstatic PUBLIC ${ZLIB_INCLUDE_DIRS})
export(TARGETS zlibstatic FILE Findzlibstatic.cmake) 
add_library(ZLIB::ZLIB ALIAS zlibstatic)
list(APPEND THIRD_PARTY_TARGETS zlibstatic)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/ZLIB-extra.cmake
        [=[
  get_target_property(ZLIB_INCLUDE_DIRS ZLIB::ZLIB INCLUDE_DIRECTORIES)
  include_directories("${ZLIB_INCLUDE_DIRS}")
  set(ZLIB_LIBRARIES ZLIB::ZLIB)
]=])

set(PNG_OPTIONS)
list(
    APPEND
    PNG_OPTIONS
        "PNG_BUILD_ZLIB OFF"
        "PNG_SHARED ${BUILD_SHARED_LIBS}"
        "PNG_TOOLS OFF"
        "PNG_TESTS OFF"
        "SKIP_INSTALL_ALL ON"
)
if (PLATFORM_TVOS)
    list(
        APPEND
        PNG_OPTIONS
            "IOS ON"
    )
endif()
CPMAddPackage(
    NAME PNG
    VERSION 1.6.42
    GITHUB_REPOSITORY pnggroup/libpng
    OPTIONS
        ${PNG_OPTIONS}
)
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

CPMAddPackage(
    NAME Boost
    VERSION 1.84.0
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
    OPTIONS
        "BOOST_ENABLE_CMAKE ON"
        "BOOST_INCLUDE_LIBRARIES filesystem\\\;algorithm\\\;serialization\\\;endian\\\;uuid\\\;regex"
)

CPMAddPackage(
    NAME glm
    GIT_TAG 33b0eb9fa336ffd8551024b1d2690e418014553b
    GITHUB_REPOSITORY g-truc/glm
    OPTIONS
        "GLM_ENABLE_CXX_20 ON"
)

CPMAddPackage(
    NAME EnTT
    VERSION 3.13.1
    GITHUB_REPOSITORY skypjack/entt
)

CPMAddPackage(
    NAME fmt
    GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
    GITHUB_REPOSITORY fmtlib/fmt
)

CPMAddPackage(
    NAME nlohmann_json
    GIT_TAG 9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03
    GITHUB_REPOSITORY nlohmann/json
)

CPMAddPackage(
    NAME nlohmann_fifo_map
    GIT_TAG d732aaf9a315415ae8fd7eb11e3a4c1f80e42a48
    GITHUB_REPOSITORY nlohmann/fifo_map
)

CPMAddPackage(
    NAME OpenSSL
    VERSION 3.8.2
    URL https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.8.2.tar.gz
    URL_HASH SHA256=6d4b8d5bbb25a1f8336639e56ec5088052d43a95256697a85c4ce91323c25954
    OPTIONS
        "LIBRESSL_APPS OFF"
        "LIBRESSL_TESTS OFF"
)
add_library(LibreSSL::Crypto ALIAS crypto)
add_library(LibreSSL::SSL ALIAS ssl)
add_library(LibreSSL::TLS ALIAS tls)
add_library(OpenSSL::Crypto ALIAS crypto)
add_library(OpenSSL::SSL ALIAS ssl)

list(APPEND CMAKE_FIND_ROOT_PATH ${LibreSSL_BINARY_DIR})
list(APPEND CMAKE_FIND_ROOT_PATH ${LibreSSL_SOURCE_DIR})
configure_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CPM_modules/FindOpenSSL.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CPM_modules/FindLibreSSL.cmake
    COPYONLY
)

CPMAddPackage(
    NAME CryptoPP-CMake
    GIT_TAG f815f6284684be6ab03af4b6c273359331c61241
    GITHUB_REPOSITORY abdes/cryptopp-cmake
    OPTIONS
        "CRYPTOPP_BUILD_TESTING OFF"
        "CRYPTOPP_INSTALL OFF"
)

set(SDL_OPTIONS)
if(PLATFORM_MACOS OR PLATFORM_IOS)
    list(
        APPEND
        SDL_OPTIONS
            "SDL_SHARED OFF"
            "SDL_STATIC ON"
    )
else()
    list(
        APPEND
        SDL_OPTIONS
            "SDL_SHARED ON"
            "SDL_STATIC OFF"
    )
endif()
list(
    APPEND
    SDL_OPTIONS
        "SDL_TEST OFF"
)
CPMAddPackage(
    NAME SDL2
    GIT_TAG 859844eae358447be8d66e6da59b6fb3df0ed778
    GITHUB_REPOSITORY libsdl-org/SDL
    OPTIONS
        ${SDL_OPTIONS}
)

CPMAddPackage(
    NAME DiligentEngine
    GIT_TAG 7d46b75850ca3fe1f7778775ea0fedb28146327e
    GITHUB_REPOSITORY DiligentGraphics/DiligentEngine
    OPTIONS
        "DILIGENT_BUILD_SAMPLES OFF"
        "DILIGENT_INSTALL_CORE OFF"
        "DILIGENT_INSTALL_FX OFF"
        "DILIGENT_INSTALL_SAMPLES OFF"
        "DILIGENT_INSTALL_TOOLS OFF"
        "DILIGENT_MSVC_RELEASE_COMPILE_OPTIONS \"/arch:SSE2\""
)

if (UI_LIBRARY STREQUAL "RmlUI")
    CPMAddPackage(
        NAME RmlUI
        GIT_TAG b8f8e2e1237a967ad5b2d2eae9a751aace428a67
        GITHUB_REPOSITORY mikke89/RmlUi
    )
endif()

CPMAddPackage(
    NAME jwt-cpp
    GIT_TAG 08bcf77a687fb06e34138e9e9fa12a4ecbe12332
    GITHUB_REPOSITORY Thalhammer/jwt-cpp
    OPTIONS
        "JWT_SSL_LIBRARY LibreSSL"
)

CPMAddPackage(
    NAME nghttp2
    GIT_TAG 4c250df3187e8b5bb1167cafc4a7a9dcc139eb02
    GITHUB_REPOSITORY nghttp2/nghttp2
    OPTIONS
        "ENABLE_LIB_ONLY ON"
        "ENABLE_DOC OFF"
        "ENABLE_STATIC_CRT ON"
)
get_target_property(NGHTTP2_INCLUDE_DIR nghttp2 INCLUDE_DIRECTORIES)
target_include_directories(nghttp2 PUBLIC ${NGHTTP2_INCLUDE_DIR})
add_library(NGHTTP2 ALIAS nghttp2)
set(NGHTTP2_LIBRARY nghttp2)
export(TARGETS nghttp2 FILE Findnghttp2.cmake) 

set(CURL_OPTIONS)
if(PLATFORM_MACOS OR PLATFORM_IOS)
    list(
        APPEND
        CURL_OPTIONS
            "BUILD_STATIC_LIBS ON"
            "BUILD_SHARED_LIBS OFF"
    )
else()
    list(
        APPEND
        CURL_OPTIONS
            "BUILD_STATIC_LIBS OFF"
            "BUILD_SHARED_LIBS ON"
    )
endif()
CPMAddPackage(
    NAME cURL
    GIT_TAG 5ce164e0e9290c96eb7d502173426c0a135ec008
    GITHUB_REPOSITORY curl/curl
    OPTIONS
        ${CURL_OPTIONS}
        "BUILD_CURL_EXE OFF"
        "CURL_USE_OPENSSL ON"
        "CURL_ENABLE_SSL ON"
        "CURL_DISABLE_INSTALL ON"
        "CURL_DISABLE_BASIC_AUTH ON"
        "CURL_DISABLE_DIGEST_AUTH ON"
        "CURL_DISABLE_KERBEROS_AUTH ON"
        "CURL_DISABLE_NEGOTIATE_AUTH ON"
        "CURL_DISABLE_FILE ON"
        "CURL_DISABLE_FTP ON"
        "CURL_DISABLE_GOPHER ON"
        "CURL_DISABLE_IMAP ON"
        "CURL_DISABLE_LDAP ON"
        "CURL_DISABLE_LDAPS ON"
        "CURL_DISABLE_MQTT ON"
        "CURL_DISABLE_NETRC ON"
        "CURL_DISABLE_NTLM ON"
        "CURL_DISABLE_POP3 ON"
        "CURL_DISABLE_RTSP ON"
        "CURL_DISABLE_SMB ON"
        "CURL_DISABLE_SMTP ON"
        "CURL_DISABLE_TELNET ON"
        "CURL_DISABLE_TFTP ON"
        "CURL_DISABLE_PROGRESS_METER ON"
        "USE_NGHTTP2 ON"
        "HAVE_SSL_SET0_WBIO 0"
)

add_subdirectory(../dependencies/freeimage ${CMAKE_CURRENT_BINARY_DIR}/dependencies/freeimage)

set(BUILD_TESTING ${BUILD_TESTING_BACKUP})