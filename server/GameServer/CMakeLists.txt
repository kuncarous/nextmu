cmake_minimum_required(VERSION 3.21.1)

option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

project(GameServerApp LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Qml Quick)

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

set(IS_CLIENT_SHARED "0")
include(../../SetPlatform.cmake)
include(Dependencies.cmake)
include(SourceTree.cmake)
add_subdirectory(../../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)

qt_add_executable(GameServerApp ${GAMESERVER_SOURCE} ${GAMESERVER_INCLUDE})

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
        icon.ico
)

target_link_libraries(GameServerApp PRIVATE
    Shared
    glm
    EnTT
    cryptopp
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
