import os
from conan import ConanFile
from conan.errors import ConanInvalidConfiguration, ConanException
from conan.tools.scm import Git
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy

class Walnut(ConanFile):
    name = "walnut"
    version = "2.0.0"
    url = "https://github.com/nisaldilshan/Walnut"
    homepage = "https://github.com/TheCherno/Walnut"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    settings = "os", "compiler", "build_type", "arch"
    
    # Optional in Conan 2, but fine to keep if you always want to build missing dependencies
    build_policy = "missing" 
    
    options = {
        "rendering_backend": ["OpenGL", "Vulkan", "WebGPU"],
        "windowing_system": ["GLFW", "SDL", "None"],
        "fPIC": [True, False],
    }
    
    default_options = {
        "rendering_backend": "Vulkan",
        "windowing_system": "SDL",
        "fPIC": True,
        "glad*:no_loader": False,
        "glad*:spec": "gl",
        "glad*:extensions": "",
        "glad*:gl_profile": "core",
        "glad*:gl_version": 4.1,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        pass

    def validate(self):
        # Validation checks move here
        valid_os = ['Macos', 'Linux', 'Windows', 'Emscripten', 'Android', 'iOS']
        if self.settings.os not in valid_os:
            raise ConanInvalidConfiguration(f"Unsupported Platform: {self.settings.os}")
            
        if self.options.windowing_system not in ["GLFW", "SDL"]:
            raise ConanInvalidConfiguration(f"Unsupported windowing system: {self.options.windowing_system}")

    def requirements(self):
        self.requires("imgui/1.92.5-docking")
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
            raise ConanInvalidConfiguration("Unsupported Platform")

        print("Using windowing system " + str(self.options.windowing_system));
        if self.options.windowing_system == "GLFW":
            self.requires("glfw/3.4")
        elif self.options.windowing_system == "SDL":
            self.requires("sdl/3.4.0")
        else:
            raise ConanInvalidConfiguration("Unsupported windowing system")

    def source(self):
        git = Git(self)
        target_ref = "master" if self.version == "latest" else f"v{self.version}"
        print(f"Attempting to checkout git ref: {target_ref}")
        try:
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
        cmake.configure()
        cmake.build()

    def package(self):
        # Conan 2's copy() tool replaces self.copy() and does not use keep_path.
        # Instead of 20 lines, we can copy all headers from the src directory while preserving structure.
        src_include_path = os.path.join(self.source_folder, "Walnut", "src")
        dst_include_path = os.path.join(self.package_folder, "include")
        copy(self, "*.h", src=src_include_path, dst=dst_include_path)

        # Copy binaries
        dst_lib_path = os.path.join(self.package_folder, "lib")
        for ext in ["*.dylib", "*.dll", "*.so", "*.a", "*.lib", "*.pdb"]:
            # Depending on your CMake output, binaries usually land in build_folder
            copy(self, ext, src=self.build_folder, dst=dst_lib_path, keep_path=False)

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

    def layout(self):
        # cmake_layout handles standard build/source folders dynamically
        cmake_layout(self)