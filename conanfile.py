from conan.errors import ConanInvalidConfiguration, ConanInvalidSystemRequirements, ConanException
from conans import ConanFile
from conan.tools.scm import Git
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain

class Walnut(ConanFile):
    name = "walnut"
    version = "1.1.0"
    url = "https://github.com/nisaldilshan/Walnut"
    homepage = "https://github.com/TheCherno/Walnut"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    settings = "os", "compiler", "build_type", "arch"
    build_policy = "missing"
    options = {
        "rendering_backend": ["OpenGL", "Vulkan", "WebGPU"],
        "windowing_system": ["GLFW", "SDL", "None"],
        'fPIC': [True, False],
    }
    default_options = {
        "rendering_backend": "Vulkan",
        "windowing_system": "GLFW",
        'fPIC': True,
        "glad:no_loader": False,
        "glad:spec": "gl",
        "glad:extensions": "",
        "glad:gl_profile": "core",
        "glad:gl_version": 4.1,
    }
    generators = "VirtualBuildEnv"

    def requirements(self):
        self.requires("imgui/latest@nisaldilshan/docking2")
        self.requires("glm/0.9.9.8")
        self.requires("stb/cci.20230920")
        print("Using rendering backend " + str(self.options.rendering_backend));
        if self.settings.os == 'Macos':
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("moltenvk/1.2.0")
            elif self.options.rendering_backend == "WebGPU":
                pass
                #self.requires("WebGPU/latest")
        elif self.settings.os == 'Linux':
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-loader/1.3.239.0")
                self.requires("vulkan-validationlayers/1.3.239.0")
            elif self.options.rendering_backend == "WebGPU":
                pass
                #self.requires("WebGPU/latest")
        elif self.settings.os == 'Windows':
            self.short_paths=True
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-loader/1.3.239.0")
                self.requires("vulkan-validationlayers/1.3.239.0")
            elif self.options.rendering_backend == "WebGPU":
                pass
                #self.requires("WebGPU/latest")
        elif self.settings.os == 'Emscripten':
            if self.options.rendering_backend == "WebGPU":
                pass
                #self.requires("WebGPU/latest")
        elif self.settings.os == 'Android':
            self.options.windowing_system = "SDL"
            if self.options.rendering_backend == "OpenGL":
                self.requires("glad/0.1.33")
            elif self.options.rendering_backend == "Vulkan":
                self.requires("vulkan-loader/1.3.239.0")
                #self.requires("vulkan-validationlayers/1.3.239.0")
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
            pass
        else:
            raise ConanInvalidConfiguration("Unsupported windowing system")

    def build_requirements(self):
        pass

    def source(self):
        git = Git(self)
        
        # Determine the target reference (branch or tag) based on version
        if self.version == "latest":
            target_ref = "master"
        else:
            # Assuming tags are named like "v0.0.1"
            target_ref = f"v{self.version}"

        print(f"Attempting to checkout git ref: {target_ref}")

        try:
            # We use --depth 1 for speed, and --branch to specify tag/branch
            clone_args = ['--depth', '1', '--branch', target_ref]
            git.clone(url=self.url + ".git", target=".", args=clone_args)
            
        except Exception as e:
            raise ConanException(
                f"\n\nERROR: Could not checkout version '{self.version}'.\n"
                f"Attempted to fetch git ref '{target_ref}' from {self.url}\n"
                f"Please verify that the tag 'v{self.version}' exists in the remote repository.\n"
                f"Original Git Error: {str(e)}\n"
            )

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
            self.cpp_info.defines = ["RENDERER_BACKEND=1"]
        elif self.options.rendering_backend == "Vulkan":
            self.cpp_info.libs = ["walnut", "walnut-graphics-vulkan"]
            self.cpp_info.defines = ["RENDERER_BACKEND=2"]
        else:
            self.cpp_info.libs = ["walnut", "walnut-graphics-webgpu", "glfw3webgpu"]
            self.cpp_info.defines = ["RENDERER_BACKEND=3"]

        if self.settings.os == 'Android':
            self.cpp_info.libs += ["App", "SDL3"]

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
