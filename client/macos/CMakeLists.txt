set(
    SOURCE
    mu_main.mm
    mu_main.cpp
)

set(
    INCLUDE
    mu_main.h
)

find_package(Vulkan REQUIRED)
get_filename_component(MOLTENVK_DIR ${Vulkan_LIBRARY} DIRECTORY)
set(MOLTENVK_LIBRARY ${MOLTENVK_DIR}/libMoltenVK.dylib)
set(MOLTENVK_LIBRARIES ${Vulkan_LIBRARY} ${MOLTENVK_LIBRARY})

set(MACOSX_BUNDLE_ICON_FILE icon.icns)
set(NEXTMU_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
add_executable(
    NextMU
    MACOSX_BUNDLE
    ${SOURCE} ${INCLUDE}
    ${NEXTMU_ICON}
)
target_link_libraries(
    NextMU
    PRIVATE
        Game
        ${MOLTENVK_PATH}
        ${MOLTENVK_LIBRARIES}
)
set_target_properties(
    NextMU
    PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks"
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

install(
    FILES "${MOLTENVK_LIBRARIES}"
    DESTINATION "$<TARGET_BUNDLE_DIR_NAME:NextMU>/Contents/Frameworks"
)