windows = import('windows')

sources += windows.compile_resources(
    'mu_main.rc',
    depend_files: files(
        'icon.ico',
        'small.ico',
    ),
)
    
sources += files(
    'stdafx.cpp',
    'mu_main.cpp'
)
includes += include_directories('.')

dependencies += [
    cpp.find_library('d3d11'),
    cpp.find_library('dxgi'),
    cpp.find_library('d3dcompiler'),
    cpp.find_library('opengl32'),
    cpp.find_library('Shlwapi'),
    cpp.find_library('Psapi'),
]
cpp_args += [
    '/WX-'
]