from conans import ConanFile, CMake, tools


class CaelumConan(ConanFile):
    name = "ogre-caelum"
    version = "0.6.3"
    license = "GNU Lesser General Public License v2.1"
    url = "https://github.com/RigsOfRods/Caelum/issues"
    description = "Caelum is a library which provides cross-platform socket abstraction"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = "main/*", "CMakeLists.txt", "Caelum.pc.in", "Doxyfile.in"


    def requirements(self):
        self.requires.add('OGRE/1.11.2@anotherfoxguy/stable')

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ['include/Caelum']
        self.cpp_info.libs = tools.collect_libs(self)
