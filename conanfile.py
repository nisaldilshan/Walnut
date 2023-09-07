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
        "use_opengl": [True, False],
    }
    default_options = {
        "use_opengl": False,
        "glad:no_loader": False,
        "glad:spec": "gl",
        "glad:extensions": "",
        "glad:gl_profile": "core",
        "glad:gl_version": 4.1
    }

    def requirements(self):
        if self.settings.os == 'Macos':
            self.requires("moltenvk/1.2.2")
        if not self.settings.os == 'Emscripten':
            self.requires("glfw/3.3.8")
            self.requires("glad/0.1.33")
        # else:
        #     self.requires("vulkan-headers/1.3.239.0")
            #self.requires("dawn/5899")

    def source(self):
        git = tools.Git()
        git.clone(self.url + ".git", "master")

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.definitions["USE_OPENGL_RENDERER"] = self.options.use_opengl
        cmake.configure()
        cmake.build()

    def package_info(self):
        if self.options.use_opengl == True:
            self.cpp_info.libs = ["walnut", "walnut-imgui", "walnut-graphics-opengl"]
        else:
            self.cpp_info.libs = ["walnut", "walnut-imgui", "walnut-graphics-vulkan"]

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
