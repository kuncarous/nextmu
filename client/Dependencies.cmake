# set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/../.cpm-cache")
include(../CPM.cmake)
include(../SetPlatform.cmake)

foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UC)
    message("CMAKE_${CONFIG_UC}_POSTFIX_BACKUP : ${CMAKE_${CONFIG_UC}_POSTFIX}")
    set("CMAKE_${CONFIG_UC}_POSTFIX_BACKUP" "${CMAKE_${CONFIG_UC}_POSTFIX}")
    list(APPEND NEXTMU_THIRD_PARTY_OPTIONS "CMAKE_${CONFIG_UC}_POSTFIX_BACKUP ${CMAKE_${CONFIG_UC}_POSTFIX}")
endforeach()

set(DEPS_MODULE_PATH
    "${CMAKE_BINARY_DIR}/DEPS_modules"
    CACHE INTERNAL ""
)
# remove old modules
file(REMOVE_RECURSE ${DEPS_MODULE_PATH})
file(MAKE_DIRECTORY ${DEPS_MODULE_PATH})
# locally added CPM modules should override global packages
set(CMAKE_MODULE_PATH "${DEPS_MODULE_PATH};${CMAKE_MODULE_PATH}")

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
    if (NOT PLATFORM_LINUX)
        link_libraries(-Wl,--undefined-version)
    endif()
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(NOT Python3_FOUND)
    message(FATAL_ERROR "Python 3 not found, please install it from Python website.")
endif()

CPMAddPackage(
    NAME ZLIB
    VERSION 1.3.1
    GITHUB_REPOSITORY madler/zlib
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
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
file(WRITE ${DEPS_MODULE_PATH}/ZLIB-extra.cmake
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
    VERSION 1.6.43
    GITHUB_REPOSITORY pnggroup/libpng
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        ${PNG_OPTIONS}
)
set_target_properties(png_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
get_target_property(PNG_INCLUDE_DIRS png_static INCLUDE_DIRECTORIES)
target_include_directories(png_static PUBLIC ${PNG_INCLUDE_DIRS})
add_library(PNG::PNG ALIAS png_static)
list(APPEND THIRD_PARTY_TARGETS png_static)
file(WRITE ${DEPS_MODULE_PATH}/PNG-extra.cmake
        [=[
  get_target_property(PNG_INCLUDE_DIRS PNG::PNG INCLUDE_DIRECTORIES)
  include_directories("${PNG_INCLUDE_DIRS}")
  set(PNG_LIBRARIES PNG::PNG)
]=])

CPMAddPackage(
    NAME Boost
    VERSION 1.86.0
    GIT_TAG tags/boost-1.86.0
    GITHUB_REPOSITORY boostorg/boost
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "BOOST_ENABLE_CMAKE ON"
        "BOOST_INCLUDE_LIBRARIES filesystem\\\;algorithm\\\;serialization\\\;endian\\\;uuid\\\;regex"
)

CPMAddPackage(
    NAME glm
    VERSION 1.0.1
    GIT_TAG tags/1.0.1
    GITHUB_REPOSITORY g-truc/glm
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "BUILD_SHARED_LIBS OFF"
        "GLM_ENABLE_CXX_20 ON"
)

CPMAddPackage(
    NAME EnTT
    VERSION 3.13.2
    GITHUB_REPOSITORY skypjack/entt
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
)

CPMAddPackage(
    NAME fmt
    VERSION 11.0.2
    GIT_TAG tags/11.0.2
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
)

CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.11.3
    GITHUB_REPOSITORY nlohmann/json
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
)

CPMAddPackage(
    NAME nlohmann_fifo_map
    VERSION 1.0.0
    GIT_TAG d732aaf9a315415ae8fd7eb11e3a4c1f80e42a48
    GITHUB_REPOSITORY nlohmann/fifo_map
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
)

CPMAddPackage(
    NAME OpenSSL
    VERSION 3.9.2
    URL https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.9.2.tar.gz
    URL_HASH SHA256=7b031dac64a59eb6ee3304f7ffb75dad33ab8c9d279c847f92c89fb846068f97
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
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
    VERSION 8.9.0
    GIT_TAG tags/CRYPTOPP_8_9_0
    GITHUB_REPOSITORY abdes/cryptopp-cmake
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
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
    VERSION 2.30.6
    GIT_TAG tags/release-2.30.6
    GITHUB_REPOSITORY libsdl-org/SDL
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        ${SDL_OPTIONS}
)
target_include_directories(SDL2 PUBLIC "${SDL2_SOURCE_DIR}/include")

CPMAddPackage(
    NAME DiligentEngine
    VERSION 2.5.5
    GITHUB_REPOSITORY DiligentGraphics/DiligentEngine
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "BUILD_SHARED_LIBS OFF"
        "DILIGENT_BUILD_SAMPLES OFF"
        "DILIGENT_INSTALL_CORE OFF"
        "DILIGENT_INSTALL_FX OFF"
        "DILIGENT_INSTALL_SAMPLES OFF"
        "DILIGENT_INSTALL_TOOLS OFF"
        "DILIGENT_MSVC_RELEASE_COMPILE_OPTIONS \"/arch:SSE2\""
        "DILIGENT_NO_FORMAT_VALIDATION ON"
)

CPMAddPackage(
    NAME jwt-cpp
    VERSION 0.7.0
    GITHUB_REPOSITORY Thalhammer/jwt-cpp
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "JWT_SSL_LIBRARY LibreSSL"
        "JWT_BUILD_EXAMPLES OFF"
)

CPMAddPackage(
    NAME nghttp2
    VERSION 1.62.1
    GITHUB_REPOSITORY nghttp2/nghttp2
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "ENABLE_LIB_ONLY ON"
        "ENABLE_DOC OFF"
        "ENABLE_STATIC_CRT ON"
)
get_target_property(NGHTTP2_INCLUDE_DIR nghttp2 INCLUDE_DIRECTORIES)
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
    VERSION 8.9.1
    GIT_TAG tags/curl-8_9_1
    GITHUB_REPOSITORY curl/curl
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
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
if(BUILD_SHARED_LIBS)
    target_compile_definitions(
        libcurl_object
        PUBLIC
            -DBUILDING_NGHTTP2=1
    )
else()
    target_compile_definitions(
        libcurl_object
        PUBLIC
            -DNGHTTP2_STATICLIB=1
    )
endif()

CPMAddPackage(
    NAME AngelScript
    VERSION 2.37.0
    URL https://www.angelcode.com/angelscript/sdk/files/angelscript_2.37.0.zip
    URL_HASH SHA256=0c52d1688016a0b2484e9ca549471c4e295df060770c57840144c64815f54f10
    SOURCE_SUBDIR angelscript/projects/cmake
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
)

# SteamAudio related
if (WIN32)
    set(IPL_OS_WINDOWS TRUE)
elseif (UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(IPL_OS_LINUX TRUE)
elseif (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(IPL_OS_MACOS TRUE)
elseif (ANDROID)
    set(IPL_OS_ANDROID TRUE)
elseif (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IPL_OS_IOS TRUE)
    if (CMAKE_XCODE_ATTRIBUTE_SDKROOT STREQUAL "iphonesimulator")
        set(IPL_IOS_SIMULATOR TRUE)
    endif()
endif()

# CPU architecture detection
if (IPL_OS_WINDOWS)
    if (CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(IPL_CPU_ARMV8 TRUE)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(IPL_CPU_X64 TRUE)
    else()
        set(IPL_CPU_X86 TRUE)
    endif()
elseif (IPL_OS_LINUX)
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(IPL_CPU_ARMV8 TRUE)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(IPL_CPU_X64 TRUE)
    else()
        set(IPL_CPU_X86 TRUE)
    endif()
elseif (IPL_OS_MACOS)
elseif (IPL_OS_ANDROID)
    if (CMAKE_ANDROID_ARCH STREQUAL "arm")
        set(IPL_CPU_ARMV7 TRUE)
    elseif (CMAKE_ANDROID_ARCH STREQUAL "arm64")
        set(IPL_CPU_ARMV8 TRUE)
    elseif (CMAKE_ANDROID_ARCH STREQUAL "x86")
        set(IPL_CPU_X86 TRUE)
    elseif (CMAKE_ANDROID_ARCH STREQUAL "x86_64")
        set(IPL_CPU_X64 TRUE)
    endif()
elseif (IPL_OS_IOS)
    set(IPL_CPU_ARMV8 TRUE)
endif()

CPMAddPackage(
    NAME SteamAudio
    VERSION 4.5.3
    URL https://github.com/ValveSoftware/steam-audio/releases/download/v4.5.3/steamaudio_4.5.3.zip
    URL_HASH SHA256=8a2648679324c38c36fa3938f3b577044b21f5a099cc582ef6bfcdb2e853e32e
    DOWNLOAD_ONLY
)

if(PLATFORM_WIN32 OR PLATFORM_LINUX OR PLATFORM_MACOS)
    set(CEF_PACKAGE_PATCH git apply --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/patches/cef-project.patch)
    CPMAddPackage(
        NAME CEF-Project
        VERSION 127.3.5+g114ea2a+chromium-127.0.6533.120
        GIT_TAG aaccb78661c390a5ce6650df8db54ea5424d431c
        GITHUB_REPOSITORY chromiumembedded/cef-project
        PATCH_COMMAND ${CEF_PACKAGE_PATCH}
        UPDATE_DISCONNECTED 1
        OPTIONS
            ${NEXTMU_THIRD_PARTY_OPTIONS}
            "BUILD_SHARED_LIBS OFF"
            "WITH_EXAMPLES OFF"
    )

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
    find_package(CEF REQUIRED)
    set_target_properties(cefclient cefsimple ceftests cef_gtest PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

    target_compile_definitions(libcef_dll_wrapper PRIVATE $<$<CONFIG:Debug>:_HAS_ITERATOR_DEBUGGING=1>)
    target_link_libraries(
            libcef_dll_wrapper
            PUBLIC
            debug ${CEF_LIB_DEBUG}
            optimized ${CEF_LIB_RELEASE}
    )

    set(POCO_PACKAGE_PATCH git apply --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/patches/poco.patch)
    CPMAddPackage(
        NAME POCO
        VERSION 1.13.3
        GIT_TAG tags/poco-1.13.3-release
        GITHUB_REPOSITORY pocoproject/poco
        PATCH_COMMAND ${POCO_PACKAGE_PATCH}
        UPDATE_DISCONNECTED 1
        OPTIONS
            ${NEXTMU_THIRD_PARTY_OPTIONS}
            "BUILD_SHARED_LIBS OFF"
            "ENABLE_NETSSL OFF"
            "ENABLE_NETSSL_WIN OFF"
            "FORCE_OPENSSL ON"
            "ENABLE_APACHECONNECTOR OFF"
            "ENABLE_DATA_MYSQL OFF"
            "ENABLE_DATA_POSTGRESQL OFF"
            "ENABLE_DATA_ODBC OFF"
            "ENABLE_ENCODINGS OFF"
            "ENABLE_ENCODINGS_COMPILER OFF"
            "ENABLE_XML OFF"
            "ENABLE_JSON OFF"
            "ENABLE_MONGODB OFF"
            "ENABLE_DATA_SQLITE OFF"
            "ENABLE_REDIS OFF"
            "ENABLE_PROMETHEUS OFF"
            "ENABLE_PDF OFF"
            "ENABLE_SEVENZIP OFF"
            "ENABLE_ZIP OFF"
            "ENABLE_CPPPARSER OFF"
            "ENABLE_POCODOC OFF"
            "ENABLE_PAGECOMPILER OFF"
            "ENABLE_PAGECOMPILER_FILE2PAGE OFF"
            "ENABLE_ACTIVERECORD OFF"
            "ENABLE_ACTIVERECORD_COMPILER OFF"
            "ENABLE_TESTS OFF"
            "ENABLE_SAMPLES OFF"
    )
    target_compile_definitions(Foundation PUBLIC POCO_NO_AUTOMATIC_LIBS)
endif()

foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UC)
    set("CMAKE_${CONFIG_UC}_POSTFIX" "${CMAKE_${CONFIG_UC}_POSTFIX_BACKUP}")
endforeach()

add_subdirectory(../dependencies/freeimage ${CMAKE_CURRENT_BINARY_DIR}/dependencies/freeimage)

set(BUILD_TESTING ${BUILD_TESTING_BACKUP})