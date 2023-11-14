from conans import ConanFile, CMake, tools

class Walnut(ConanFile):
    name = "walnut"
    version = "latest"
    url = "https://github.com/nisaldilshan/Walnut"
    homepage = "https://github.com/TheCherno/Walnut"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    settings = "os", "compiler", "build_type", "arch"
    requires = ("stb/20200203", "imgui/latest@nisaldilshan/docking", "glm/0.9.9.8")
    generators = "cmake"
    build_policy = "missing"
    options = {
        "rendering_backend": ["OpenGL", "Vulkan", "WebGPU"],
    }
    default_options = {
        "rendering_backend": "WebGPU",
        "glad:no_loader": False,
        "glad:spec": "gl",
        "glad:extensions": "",
        "glad:gl_profile": "core",
        "glad:gl_version": 4.1
    }

    def requirements(self):
        print("Using rendering backend " + str(self.options.rendering_backend));
        if self.settings.os == 'Macos':
            self.requires("glfw/3.3.8")
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("moltenvk/1.2.2")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        elif self.settings.os == 'Linux':
            self.requires("glfw/3.3.8")
            if self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-headers/1.3.239.0")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        elif self.settings.os == 'Windows':
            self.requires("glfw/3.3.8")
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-headers/1.3.239.0")
            elif self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        elif self.settings.os == 'Emscripten':
            if self.options.rendering_backend == "WebGPU":
                self.requires("WebGPU/latest@nisaldilshan/testing")
        else:
            logging.critical("Unsupported Platform : " + self.settings.os)

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
            self.cpp_info.libs = ["walnut", "walnut-imgui", "walnut-graphics-opengl"]
        elif self.options.rendering_backend == "Vulkan":
            self.cpp_info.libs = ["walnut", "walnut-imgui", "walnut-graphics-vulkan"]
        else:
            self.cpp_info.libs = ["walnut", "walnut-imgui", "walnut-graphics-webgpu"]


    def package(self):
        self.copy(pattern="Walnut/src/Walnut/EntryPoint.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Application.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Image.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/ImageFormat.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Layer.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Random.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Timer.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/RenderingBackend.h", dst="include/Walnut", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Input/Input.h", dst="include/Walnut/Input", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/Input/KeyCodes.h", dst="include/Walnut/Input", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/VulkanImage.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="Walnut/src/Walnut/GraphicsAPI/OpenGLImage.h", dst="include/Walnut/GraphicsAPI", keep_path=False)
        self.copy(pattern="*.a", dst="lib", keep_path=False)
        self.copy(pattern="*.so", dst="lib", keep_path=False)
