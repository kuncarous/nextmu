from conan import ConanFile
from conan.tools.cmake import cmake_layout

class GameServerConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    
    def requirements(self):
        self.requires("grpc/1.54.3")
    
    def layout(self):
        cmake_layout(self)
