from conan.errors import ConanInvalidSystemRequirements
from conan.errors import ConanInvalidConfiguration
from conans import ConanFile, CMake, tools

class Walnut(ConanFile):
    name = "walnut"
    version = "latest"
    url = "https://github.com/nisaldilshan/Walnut"
    homepage = "https://github.com/TheCherno/Walnut"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    settings = "os", "compiler", "build_type", "arch"
    requires = ("stb/20200203", "imgui/latest@nisaldilshan/docking", "glm/0.9.9.8", "tinyobjloader/2.0.0-rc10")
    generators = "cmake"
    build_policy = "missing"
    options = {
        "rendering_backend": ["OpenGL", "Vulkan", "WebGPU"],
        'fPIC': [True, False]
    }
    default_options = {
        "rendering_backend": "WebGPU",
        'fPIC': True,
        "glad:no_loader": False,
        "glad:spec": "gl",
        "glad:extensions": "",
        "glad:gl_profile": "core",
        "glad:gl_version": 4.1
    }

    def requirements(self):
        print("Using rendering backend " + str(self.options.rendering_backend));
        if self.settings.os == 'Macos':
            self.requires("glfw/3.4")
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("moltenvk/1.2.9")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        elif self.settings.os == 'Linux':
            self.requires("glfw/3.4")
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-headers/1.3.239.0")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        elif self.settings.os == 'Windows':
            self.short_paths=True
            self.requires("glfw/3.4")
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                raise ConanInvalidConfiguration("Vulkan renderer is not implemented in Windows")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        elif self.settings.os == 'Emscripten':
            if self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        else:
            raise ConanInvalidSystemRequirements("Unsupported Platform")

    def source(self):
        git = tools.Git()
        git.clone(self.url + ".git", "master")

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.definitions["RENDERER"] = self.options.rendering_backend
        cmake.configure()
        cmake.build()

    def package_info(self):
        if self.options.rendering_backend == "OpenGL":
            self.cpp_info.libs = ["walnut", "walnut-graphics-opengl"]
        elif self.options.rendering_backend == "Vulkan":
            self.cpp_info.libs = ["walnut", "walnut-graphics-vulkan"]
        else:
            self.cpp_info.libs = ["walnut", "walnut-graphics-webgpu"]


    def package(self):
        self.copy(pattern="Walnut/src/Walnut/EntryPoint.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Application.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Image.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/ImageFormat.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Layer.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Random.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Timer.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/RenderingBackend.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/ExportConfig.h", dst="include/Walnut", keep_path=False)
        # imgui_header = str(self.deps_cpp_info["imgui"].include_paths[0]) + "/imgui_impl_glfw.h"
        # print("logs " + imgui_header);
        # self.copy(pattern="/Users/nisal/.conan/data/imgui/latest/nisaldilshan/docking/package/5c91a8b3216ac0ed6c294940851231d60f21ef32/include/imgui_impl_glfw.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Input/Input.h", dst="include/Walnut/Input", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Input/KeyCodes.h", dst="include/Walnut/Input", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/Vulkan/VulkanImage.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/OpenGL/OpenGLImage.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUImage.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/Vulkan/VulkanGraphics.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/OpenGL/OpenGLGraphics.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="*.dylib", dst="lib", keep_path=False)
        self.copy(pattern="*.dll", dst="lib", keep_path=False)
        self.copy(pattern="*.so", dst="lib", keep_path=False)
        self.copy(pattern="*.a", dst="lib", keep_path=False)
        self.copy(pattern="*.lib", dst="lib", keep_path=False)
        self.copy(pattern="*.pdb", dst="lib", keep_path=False)
