set(
    RESOURCE
    icon.ico
    small.ico
)

set(
    SOURCE
    mu_main.cpp
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_executable(NextMU ${SOURCE} ${RESOURCE})
target_link_libraries(
    NextMU
    PRIVATE
        Game
)
add_dependencies(NextMU NextMUBrowser)
#add_custom_command(
#    TARGET NextMU POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:NextMU> $<TARGET_FILE_DIR:NextMU>
#    COMMAND_EXPAND_LISTS
#)
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
                ${NOESISGUI_BINARY_DIR}/libNoesis.so
                $<TARGET_FILE_DIR:NextMU>
    )
endif()

source_group("src" FILES ${SOURCE})
source_group("resources" FILES ${RESOURCE})