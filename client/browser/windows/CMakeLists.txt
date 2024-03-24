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
add_executable(NextMUBrowser WIN32 ${SOURCE} ${INCLUDE} ${RESOURCE})
target_include_directories(NextMUBrowser PRIVATE ${CEF_INCLUDE_PATH})
target_link_libraries(
    NextMUBrowser
    PRIVATE
        libcef_dll_wrapper
        d3d11.lib
        d3d12.lib
        ninput.lib
        winmm.lib
        dxgi.lib
        shcore.lib
        dxguid.lib
)
add_custom_command(
    TARGET NextMUBrowser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CEF_INCLUDE_PATH}/Resources $<TARGET_FILE_DIR:NextMUBrowser>/cef_resources
    COMMAND_EXPAND_LISTS
)
add_custom_command(
    TARGET NextMUBrowser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CEF_INCLUDE_PATH}/Resources/icudtl.dat $<TARGET_FILE_DIR:NextMUBrowser>
    COMMAND_EXPAND_LISTS
)
foreach(CEF_BINARY_TO_COPY ${CEF_BINARY_FILES})
    add_custom_command(
        TARGET NextMUBrowser POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CEF_BINARY_DIR}/${CEF_BINARY_TO_COPY} $<TARGET_FILE_DIR:NextMUBrowser>
        COMMAND_EXPAND_LISTS
    )
endforeach()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("resources" FILES ${RESOURCE})