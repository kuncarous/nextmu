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
    link_libraries(-Wl,--undefined-version)
endif()

CPMAddPackage(
    NAME ZLIB
    VERSION 1.3.1
    GIT_TAG 51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf
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
    VERSION 1.84.0
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "BOOST_ENABLE_CMAKE ON"
        "BOOST_INCLUDE_LIBRARIES filesystem\\\;algorithm\\\;serialization\\\;endian\\\;uuid\\\;regex"
)

CPMAddPackage(
    NAME glm
    VERSION 1.0.0
    GIT_TAG 33b0eb9fa336ffd8551024b1d2690e418014553b
    GITHUB_REPOSITORY g-truc/glm
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "BUILD_SHARED_LIBS OFF"
        "GLM_ENABLE_CXX_20 ON"
)

CPMAddPackage(
    NAME EnTT
    VERSION 3.13.1
    GITHUB_REPOSITORY skypjack/entt
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
)

CPMAddPackage(
    NAME fmt
    VERSION 10.2.1
    GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
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
    VERSION 3.8.2
    URL https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.8.2.tar.gz
    URL_HASH SHA256=6d4b8d5bbb25a1f8336639e56ec5088052d43a95256697a85c4ce91323c25954
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
    GIT_TAG f815f6284684be6ab03af4b6c273359331c61241
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
    VERSION 2.30.1
    GIT_TAG 5adbf3765a57dc5931c2a3137390bfee2370c945
    GITHUB_REPOSITORY libsdl-org/SDL
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        ${SDL_OPTIONS}
)

CPMAddPackage(
    NAME DiligentEngine
    VERSION 2.5.4
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
)

CPMAddPackage(
    NAME jwt-cpp
    VERSION 0.7.0
    GITHUB_REPOSITORY Thalhammer/jwt-cpp
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "JWT_SSL_LIBRARY LibreSSL"
)

CPMAddPackage(
    NAME nghttp2
    VERSION 1.60.0
    GITHUB_REPOSITORY nghttp2/nghttp2
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
        "ENABLE_LIB_ONLY ON"
        "ENABLE_DOC OFF"
        "ENABLE_STATIC_CRT ON"
)
get_target_property(NGHTTP2_INCLUDE_DIR nghttp2_static INCLUDE_DIRECTORIES)
add_library(NGHTTP2 ALIAS nghttp2_static)
set(NGHTTP2_LIBRARY nghttp2_static)
export(TARGETS nghttp2_static FILE Findnghttp2.cmake) 

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
    VERSION 8.6.0
    GIT_TAG 5ce164e0e9290c96eb7d502173426c0a135ec008
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

CPMAddPackage(
    NAME AngelScript
    VERSION 2.36.1
    URL http://angelcode.com/angelscript/sdk/files/angelscript_2.36.1.zip
    URL_HASH SHA256=58bb749af9c7e386304705f4e6e627ae41dfe03e0b6a73c3d0d2e017c4fc948f
    SOURCE_SUBDIR angelscript/projects/cmake
    OPTIONS
        ${NEXTMU_THIRD_PARTY_OPTIONS}
)

set(CEF_PACKAGE_PATCH git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/cef-project.patch)
CPMAddPackage(
    NAME CEF-Project
    VERSION 122.1.13+gde5b724+chromium-122.0.6261.130
    GIT_TAG master
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

target_compile_definitions(libcef_dll_wrapper PRIVATE $<$<CONFIG:Debug>:_HAS_ITERATOR_DEBUGGING=1>)
target_link_libraries(
    libcef_dll_wrapper
    PUBLIC
        debug ${CEF_LIB_DEBUG}
        optimized ${CEF_LIB_RELEASE}
)

if(PLATFORM_WIN32 OR PLATFORM_LINUX OR PLATFORM_MACOS)
    set(POCO_PACKAGE_PATCH git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/poco.patch)
    CPMAddPackage(
        NAME POCO
        VERSION 1.13.2
        GIT_TAG a8aea338d211d6dbce75f5f07d70c449b94f1605
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