from conans import ConanFile, tools

class MoltenVKConan(ConanFile):
    name = 'MoltenVK'
    version = '1.2.9'
    license = 'Apache-2.0'
    author = 'The Khronos Group'
    description = 'MoltenVK is a layered implementation of Vulkan graphics, which is built up on Metal graphics framework on macOS, iOS, tvOS, and visionOS.'
    url = 'https://github.com/KhronosGroup/MoltenVK'
    topics = ('apple', 'metal', 'moltenvk', 'vulkan')
    settings = 'os', 'compiler', 'build_type', 'arch'

    def requirements(self):
        pass

    def build_requirements(self):
        pass

    def source(self):
        self.run("git clone --depth=1 " + self.url + ".git --branch {} .".format(self.options.branch))

    def configure(self):
        pass

    def config_options(self):
        pass

    def generate(self):
        pass

    def build(self):
        pass

    def package_id(self):
        pass

    def package(self):
        self.copy(pattern="_deps/dawn-build/gen/include/dawn/webgpu.h", dst="include/webgpu", keep_path=False)
        self.copy(pattern="include/webgpu/webgpu.hpp", dst="include/webgpu", keep_path=False)
        self.copy(pattern="*libwebgpu_dawn.a", dst="lib", keep_path=False)
        self.copy(pattern="*libwebgpu_dawn.dylib", dst="lib", keep_path=False)

    def package_info(self):
        pass

