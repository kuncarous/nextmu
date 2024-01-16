sources += files(
    'lib/mu_main.cpp'
)
includes += include_directories('lib')

dependencies += [
    cpp.find_library('d3d11'),
    cpp.find_library('dxgi'),
    cpp.find_library('d3dcompiler'),
    cpp.find_library('opengl32'),
    cpp.find_library('Shlwapi'),
    cpp.find_library('Psapi'),
]
compiler_args += [
    '-DANDROID',
]