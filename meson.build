project(
    'nextmu',
    'c', 'cpp',
    version: '0.1',
    meson_version : '>= 1.3',
    default_options: ['warning_level=3', 'cpp_std=vc++latest']
)

cmake = import('cmake')
cpp = meson.get_compiler('cpp')

compiler_args = [
    '-DFREEIMAGE_LIB'
]
link_args = []
c_args = []
cpp_args = []
sources = []
includes = []
dependencies = []

# target platform (https://mesonbuild.com/Reference-tables.html#operating-system-names)
host_system = host_machine.system()

# if target platform is darwin we check if we are targeting iOS
if host_system == 'darwin'
    ios_test_code = '''#include <TargetConditionals.h>
    #if ! TARGET_OS_IPHONE
    #error "Not iOS/tvOS/watchOS/iPhoneSimulator"
    #endif'''
    if cc.compiles(ios_test_code, name : 'building for iOS')
        host_system = 'ios'
    endif

    # add objective-c as language to the project
    add_languages('objc', native: false, required: true)
endif

if host_system == 'windows'
    subdir('client/windows')
endif

subdir('client/game')
subdir('shared')

add_project_arguments(cpp.get_supported_arguments(compiler_args), language: 'cpp')

dependencies_name = ['cryptopp', 'glm', 'fmt', 'entt', 'sdl2', 'nlohmann_json', 'freeimage']
dependencies += dependency('gl')

buildtype = get_option('buildtype')
cmake_buildtype = ''
if buildtype == 'release'
    cmake_buildtype = 'Release'
elif buildtype == 'debugoptimized'
    cmake_buildtype = 'RelWithDebInfo'
else
    cmake_buildtype = 'Debug'
endif

# Boost requires a special parsing since it has multiples dependencies
boost_dependencies = ['boost_algorithm', 'boost_serialization']
boost_options = cmake.subproject_options()
boost_options.add_cmake_defines({
    'CMAKE_BUILD_TYPE': cmake_buildtype,
    'BOOST_INCLUDE_LIBRARIES': 'algorithm;serialization'
})
boost_proj = cmake.subproject(
    'boost',
    options: boost_options
)
foreach dependency_name : boost_dependencies
    dependencies += boost_proj.get_variable('@0@_dep'.format(dependency_name))
endforeach

# We don't want to use already existing dependencies since we want to avoid issues so we are going to always use subprojects
foreach dependency_name : dependencies_name
    proj = subproject(dependency_name)
    dependencies += proj.get_variable('@0@_dep'.format(dependency_name))
endforeach

# Diligent Engine requires a special parsing since it has multiples dependencies
diligent_dependencies = [
    'glew_static',
    'DiligentFX',
    'Diligent_GraphicsTools',
    'Diligent_GraphicsAccessories',
    'Diligent_GraphicsEngineVk_static',
    'Diligent_GraphicsEngineNextGenBase',
    'Diligent_GraphicsEngineOpenGL_static',
    'Diligent_ShaderTools',
    'Diligent_HLSL2GLSLConverterLib',
    'spirv_cross_core',
    'SPIRV',
    'SPIRV_Tools_static',
    'SPIRV_Tools_opt',
    'glslang',
    'MachineIndependent',
    'GenericCodeGen',
    'OGLCompiler',
    'OSDependent',
    'Diligent_GraphicsEngine',
    'Diligent_Common',
    'Diligent_BasicPlatform',
    'Diligent_Primitives'
]
diligent_options = cmake.subproject_options()
diligent_options.add_cmake_defines({
    'CMAKE_BUILD_TYPE': cmake_buildtype,
    'DILIGENT_BUILD_SAMPLES': false
})
diligent_proj = cmake.subproject(
    'diligent_engine',
    options: diligent_options
)
foreach dependency_name : diligent_dependencies
    dependencies += diligent_proj.get_variable('@0@_dep'.format(dependency_name))
endforeach

if host_system == 'windows'
    diligent_windows_dependencies = [
        'Diligent_GraphicsEngineD3D11_static',
        'Diligent_GraphicsEngineD3D12_static',
        'Diligent_GraphicsEngineD3DBase',
        'Diligent_Win32Platform',
    ]
    foreach dependency_name : diligent_windows_dependencies
        dependencies += diligent_proj.get_variable('@0@_dep'.format(dependency_name))
    endforeach
endif

if host_system == 'windows'
    executable(
        'nextmu',
        sources,
        dependencies: dependencies,
        cpp_pch: 'client/windows/stdafx.h',
        include_directories: includes,
        c_args: c_args,
        cpp_args: cpp_args,
        link_args: link_args,
        win_subsystem: 'windows',
    )
endif