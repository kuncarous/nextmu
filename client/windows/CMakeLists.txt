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
add_custom_command(
    TARGET NextMU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:NextMU> $<TARGET_FILE_DIR:NextMU>
    COMMAND_EXPAND_LISTS
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("resources" FILES ${RESOURCE})