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
        Boost::algorithm
        Boost::serialization
        fmt
        SDL2
        nlohmann_json
)
target_precompile_headers(NextMU PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/stdafx.h)
target_include_directories(NextMU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})