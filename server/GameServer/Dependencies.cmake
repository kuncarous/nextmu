# set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/../../.cpm-cache")
include(../../CPM.cmake)
include(../../SetPlatform.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-unsafe-buffer-usage)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unsafe-buffer-usage)
    link_libraries(-Wl,--undefined-version)
endif()

find_package(gRPC CONFIG REQUIRED)

CPMAddPackage(
    NAME Boost
    VERSION 1.84.0
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
    OPTIONS
        "BOOST_ENABLE_CMAKE ON"
        "BOOST_INCLUDE_LIBRARIES filesystem\\\;algorithm\\\;serialization\\\;uuid\\\;regex"
)

CPMAddPackage(
    NAME glm
    VERSION 1.0.0
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
    VERSION 10.2.1
    GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
    GITHUB_REPOSITORY fmtlib/fmt
)

CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.11.3
    GITHUB_REPOSITORY nlohmann/json
)

CPMAddPackage(
    NAME nlohmann_fifo_map
    VERSION 1.0.0
    GIT_TAG d732aaf9a315415ae8fd7eb11e3a4c1f80e42a48
    GITHUB_REPOSITORY nlohmann/fifo_map
)

CPMAddPackage(
    NAME LibreSSL
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

CPMAddPackage(
    NAME CryptoPP-CMake
    VERSION 8.9.0
    GIT_TAG f815f6284684be6ab03af4b6c273359331c61241
    GITHUB_REPOSITORY abdes/cryptopp-cmake
    OPTIONS
        "CRYPTOPP_BUILD_TESTING OFF"
        "CRYPTOPP_INSTALL OFF"
)

CPMAddPackage(
    NAME jwt-cpp
    VERSION 0.7.0
    GITHUB_REPOSITORY Thalhammer/jwt-cpp
    OPTIONS
        "JWT_SSL_LIBRARY LibreSSL"
)