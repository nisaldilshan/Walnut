from conans import ConanFile, tools
from conan.tools.files import download

class MoltenVKConan(ConanFile):
    name = 'moltenvk'
    version = '1.2.9'
    license = 'Apache-2.0'
    author = 'The Khronos Group'
    description = 'MoltenVK is a layered implementation of Vulkan graphics, which is built up on Metal graphics framework on macOS, iOS, tvOS, and visionOS.'
    url = 'https://github.com/KhronosGroup/MoltenVK'
    topics = ('apple', 'metal', 'moltenvk', 'vulkan')
    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'shared': [True, False]
    }
    default_options = {
        'shared': False
    }

    def requirements(self):
        pass

    def build_requirements(self):
        pass

    def source(self):
        download(self, self.url + '/releases/download/v' + self.version + '/MoltenVK-macos.tar', 'MoltenVK-macos.tar')
        tools.unzip('MoltenVK-macos.tar', strip_root=True)

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
        self.copy(pattern="MoltenVK/include/MoltenVK/*", dst="include/MoltenVK/", keep_path=False)
        self.copy(pattern="MoltenVK/include/vk_video/*", dst="include/vk_video/", keep_path=False)
        self.copy(pattern="MoltenVK/include/vulkan/*", dst="include/vulkan/", keep_path=False)

        if self.options.shared:
            self.copy(pattern="MoltenVK/dynamic/dylib/macOS/*", dst="lib/", keep_path=False)
        else:
            self.copy(pattern="MoltenVK/static/MoltenVK.xcframework/*", dst="lib/", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["MoltenVK"]
        self.cpp_info.frameworks = ["Metal", "Foundation", "CoreFoundation", "QuartzCore", "IOSurface", "CoreGraphics"]
        if self.settings.os == "Macos":
            self.cpp_info.frameworks.extend(["AppKit", "IOKit"])
        elif self.settings.os in ["iOS", "tvOS"]:
            self.cpp_info.frameworks.append("UIKit")

