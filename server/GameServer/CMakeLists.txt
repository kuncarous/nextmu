cmake_minimum_required(VERSION 3.21.1)

option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(BUILD_SHARED_LIBS OFF)

project(GameServerApp LANGUAGES CXX)
if (MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/EHsc>)
else()
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fexceptions>)
endif()

include(../../SetPlatform.cmake)
include(Dependencies.cmake)
include(SourceTree.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Qml Quick)

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

set(IS_CLIENT_SHARED "0")
add_subdirectory(../../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)

set(MACOSX_BUNDLE_ICON_FILE icon.icns)

if(WIN32)
    set(GAMESERVER_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/mu_main.rc")
elseif(APPLE)
    set(GAMESERVER_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns")
    set_source_files_properties(
            ${GAMESERVER_RESOURCES} PROPERTIES
           MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

qt_add_executable(GameServerApp WIN32 MACOSX_BUNDLE ${GAMESERVER_SOURCE} ${GAMESERVER_INCLUDE} ${GAMESERVER_RESOURCES})

list(
    APPEND PRIVATE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/imports/GameServerBackend
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Detour/Include
)
target_precompile_headers(GameServerApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/mu_precompiled.h)
target_include_directories(GameServerApp PRIVATE ${PRIVATE_INCLUDES})

qt_add_resources(GameServerApp "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
        resources/icon.ico
        resources/icon.icns
)

target_link_libraries(GameServerApp PRIVATE
    Shared
    glm::glm
    EnTT
    cryptopp
    grpc++
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

if (BUILD_QDS_COMPONENTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/qmlcomponents)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

if (LINK_INSIGHT)
    include(${CMAKE_CURRENT_SOURCE_DIR}/insight)
endif ()

include(GNUInstallDirs)
install(TARGETS GameServerApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# make IDEs aware of the QML import path
set(QML_IMPORT_PATH ${PROJECT_BINARY_DIR}/qml CACHE PATH
    "Path to the custom QML components defined by the project")

qt_generate_deploy_qml_app_script(
    TARGET GameServerApp
    OUTPUT_SCRIPT deploy_script
    NO_COMPILER_RUNTIME
    NO_UNSUPPORTED_PLATFORM_ERROR
    MACOS_BUNDLE_POST_BUILD
)

install(TARGETS GameServerApp DESTINATION bin)
install(SCRIPT deploy_script)
