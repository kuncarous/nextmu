set(
    INCLUDE
	include/stdafx.h
)

set(
    SOURCE
    src/mu_main.cpp
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_library(NextMU SHARED ${SOURCE} ${INCLUDE})
target_link_libraries(
    NextMU
    PRIVATE
        Game
        Boost::headers
        Boost::filesystem
        Boost::serialization
        Boost::uuid
        Boost::regex
        fmt::fmt
        SDL2
        nlohmann_json::nlohmann_json
        fifo_map::fifo_map
)
target_precompile_headers(NextMU PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/stdafx.h)
target_include_directories(NextMU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(UI_LIBRARY STREQUAL "NoesisGUI")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../NoesisGUI.cmake)
    add_custom_command(
        TARGET NextMU POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${NOESISGUI_BINARY_DIR}/libNoesis.so
                $<TARGET_FILE_DIR:NextMU>
    )
endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})