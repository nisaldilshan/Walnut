from conan.errors import ConanInvalidSystemRequirements
from conan.errors import ConanInvalidConfiguration
from conans import ConanFile, tools
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain

class Walnut(ConanFile):
    name = "walnut"
    version = "latest"
    url = "https://github.com/nisaldilshan/Walnut"
    homepage = "https://github.com/TheCherno/Walnut"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    settings = "os", "compiler", "build_type", "arch"
    requires = ("stb/20200203", "imgui/latest@nisaldilshan/docking", "glm/0.9.9.8", "tinyobjloader/2.0.0-rc10")
    build_policy = "missing"
    options = {
        "rendering_backend": ["OpenGL", "Vulkan", "WebGPU"],
        "windowing_system": ["GLFW", "SDL", "None"],
        'fPIC': [True, False]
    }
    default_options = {
        "rendering_backend": "WebGPU",
        "windowing_system": "GLFW",
        'fPIC': True,
        "glad:no_loader": False,
        "glad:spec": "gl",
        "glad:extensions": "",
        "glad:gl_profile": "core",
        "glad:gl_version": 4.1
    }
    generators = "VirtualBuildEnv"

    def requirements(self):
        print("Using rendering backend " + str(self.options.rendering_backend));
        if self.settings.os == 'Macos':
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("moltenvk/1.2.0")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest")
        elif self.settings.os == 'Linux':
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-headers/1.3.239.0")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest")
        elif self.settings.os == 'Windows':
            self.short_paths=True
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-loader/1.3.239.0")
                self.requires("vulkan-validationlayers/1.3.239.0")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest")
        elif self.settings.os == 'Emscripten':
            if self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        elif self.settings.os == 'Android':
            self.options.windowing_system = "SDL"
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
        elif self.settings.os == 'iOS':
            self.options.windowing_system = "SDL"
            if self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-headers/1.3.239.0")
                self.requires("moltenvk/1.2.0")
        else:
            raise ConanInvalidSystemRequirements("Unsupported Platform")

        print("Using windowing system " + str(self.options.windowing_system));
        if self.options.windowing_system == "GLFW":
            self.requires("glfw/3.4")
        elif self.options.windowing_system == "SDL":
            self.requires("sdl/2.30.5")
        else:
            pass

    def build_requirements(self):
        pass

    def source(self):
        git = tools.Git()
        git.clone(self.url + ".git", "master")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["RENDERER"] = self.options.rendering_backend
        tc.variables["WINDOWING_SYSTEM"] = self.options.windowing_system
        if self.settings.os == 'iOS':
            tc.generator = "Xcode"
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()

    def package_info(self):
        if self.options.rendering_backend == "OpenGL":
            self.cpp_info.libs = ["walnut", "walnut-graphics-opengl"]
        elif self.options.rendering_backend == "Vulkan":
            self.cpp_info.libs = ["walnut", "walnut-graphics-vulkan"]
        else:
            self.cpp_info.libs = ["walnut", "walnut-graphics-webgpu", "glfw3webgpu"]


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
        self.copy(pattern="Walnut/src/Walnut/WindowHandle.h", dst="include/Walnut", keep_path=False)
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

    def layout(self):
        self.folders.source = "."
        self.folders.build = "build"
        self.folders.generators = "build"
        self.folders.imports = self.folders.build
