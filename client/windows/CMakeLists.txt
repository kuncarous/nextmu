set(
    RESOURCE
	mu_main.rc
    icon.ico
    small.ico
)

set(
    INCLUDE
    stdafx.h
    targetver.h
    resource.h
)

set(
    SOURCE
    stdafx.cpp
    mu_main.cpp
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_executable(NextMU WIN32 ${SOURCE} ${INCLUDE} ${RESOURCE})
target_link_libraries(
    NextMU
    PRIVATE
        Game
        d3d11.lib
        d3d12.lib
        ninput.lib
        winmm.lib
        dxgi.lib
        shcore.lib
        dxguid.lib
)
add_dependencies(NextMU NextMUBrowser)
add_custom_command(
    TARGET NextMU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:NextMU> $<TARGET_FILE_DIR:NextMU>
    COMMAND_EXPAND_LISTS
)
add_custom_command(
    TARGET NextMU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:NextMUBrowser> $<TARGET_FILE_DIR:NextMU>
    COMMAND_EXPAND_LISTS
)
add_custom_command(
    TARGET NextMU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:CURL::libcurl> $<TARGET_FILE_DIR:NextMU>
    COMMAND_EXPAND_LISTS
)
if(UI_LIBRARY STREQUAL "NoesisGUI")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../NoesisGUI.cmake)
    add_custom_command(
        TARGET NextMU POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${NOESISGUI_BINARY_DIR}/Noesis.dll
                $<TARGET_FILE_DIR:NextMU>
    )
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../SteamAudio.cmake)
file(GLOB STEAMAUDIO_BINARY_FILES "${STEAMAUDIO_LIBRARY_DIR}/*.dll")
foreach(STEAMAUDIO_DLL ${STEAMAUDIO_BINARY_FILES})
    add_custom_command(
        TARGET NextMU POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${STEAMAUDIO_DLL} $<TARGET_FILE_DIR:NextMU>
        COMMAND_EXPAND_LISTS
    )
endforeach()

ADD_CUSTOM_COMMAND(
    TARGET NextMU
    POST_BUILD
    COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\dpiawareness.manifest\" -inputresource:\"$<TARGET_FILE:NextMU>\"\;\#1 -outputresource:\"$<TARGET_FILE:NextMU>\"\;\#1
    COMMENT "Adding display aware manifest..." 
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("resources" FILES ${RESOURCE})