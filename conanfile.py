from conans import ConanFile, CMake, tools

class Walnut(ConanFile):
    name = "walnut"
    version = "latest"
    url = "https://github.com/nisaldilshan/Walnut"
    homepage = "https://github.com/TheCherno/Walnut"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    settings = "os", "compiler", "build_type", "arch"
    requires = ("glfw/3.3.2", "stb/20200203", "imgui/latest@nisaldilshan/docking", "glm/0.9.9.8", "moltenvk/1.2.2")
    generators = "cmake"
    build_policy = "missing"

    def source(self):
        git = tools.Git()
        git.clone(self.url + ".git", "master")

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()

    # def package(self):
    #     self.copy(pattern="LICENSE.txt", dst="licenses", src=self._source_subfolder)
    #     self.copy(pattern="backends/imgui_impl_*", dst="include", src=self._source_subfolder, keep_path=False)
    #     self.copy(pattern="imconfig.h", dst="include", src=self._source_subfolder, keep_path=False)
    #     self.copy(pattern="imgui.h", dst="include", src=self._source_subfolder, keep_path=False)
    #     self.copy(pattern="imgui.cpp", dst="include", src=self._source_subfolder, keep_path=False)
    #     self.copy(pattern="imgui_*", dst="include", src=self._source_subfolder, keep_path=False)
    #     self.copy(pattern="imgui_*", dst="include", src=self._source_subfolder, keep_path=False)
    #     self.copy(pattern="imstb_*", dst="include", src=self._source_subfolder, keep_path=False)