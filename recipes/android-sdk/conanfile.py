import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import copy, get

required_conan_version = ">=2.0"


class AndroidSdk(ConanFile):
    """The parts of the Android SDK that package and install an APK, downloaded from Google.

    The SDK's license does not allow redistributing it, so this recipe lives in the Walnut repository instead
    of ConanCenter: it downloads the SDK on the machine that uses it, once its user has accepted the license,
    and the package is never uploaded. Export it before building for Android:

        conan export recipes/android-sdk

    The package has the layout sdkmanager creates, so its folder works as ANDROID_HOME:
      build-tools/<version>   aapt2, d8, zipalign and apksigner
      platforms/android-<N>   android.jar of the platform the app is compiled against
      platform-tools          adb
    """

    name = "android-sdk"
    # The version of the build tools
    version = "35.0.0"
    description = "Android SDK build tools, platform and platform tools, downloaded from Google"
    license = "LicenseRef-Android-Software-Development-Kit-License"
    homepage = "https://developer.android.com/studio"
    url = "https://github.com/nisaldilshan/Walnut"
    package_type = "application"
    settings = "os", "arch"
    # The SDK may be downloaded for your own use but not redistributed
    upload_policy = "skip"
    build_policy = "missing"

    # API level of the platform
    _platform = "35"
    _platform_tools_version = "37.0.1"

    # Archive file names and SHA-1 checksums from Google's repository index,
    # https://dl.google.com/android/repository/repository2-3.xml (packages build-tools;35.0.0, platforms;android-35
    # and platform-tools 37.0.1).
    _url = "https://dl.google.com/android/repository/"
    _build_tools = {
        "Linux": ("build-tools_r35_linux.zip", "2cfaa0bbb2336e9ec18ed3ecea84fa2e2af607bc"),
        "Macos": ("build-tools_r35_macosx.zip", "93ab8ce91230e067b5add4bfa79919c52b27f072"),
        "Windows": ("build-tools_r35_windows.zip", "af059bb67cf7786f45ee0db85e2d24985df1b4b6"),
    }
    _platform_tools = {
        "Linux": ("platform-tools_r37.0.1-linux.zip", "477254aa5f903c15cf51001717bdf347fb6b53e0"),
        "Macos": ("platform-tools_r37.0.1-darwin.zip", "6ae73f4de6452dc57e62ec02b68eed92a4c21661"),
        "Windows": ("platform-tools_r37.0.1-win.zip", "e03e78b1d80b396f1c3358e31251cb31740e1110"),
    }
    _platform_archive = ("platform-35_r02.zip", "0bb560a90a7a2cbd0dd8348224d518b638fe7949")

    @property
    def _build_tools_folder(self):
        return os.path.join("build-tools", self.version)

    @property
    def _platform_folder(self):
        return os.path.join("platforms", f"android-{self._platform}")

    def validate(self):
        if str(self.settings.os) not in self._build_tools:
            raise ConanInvalidConfiguration(f"Google publishes the Android SDK for Linux, macOS and Windows, not {self.settings.os}")
        if self.settings.os == "Linux" and self.settings.arch != "x86_64":
            raise ConanInvalidConfiguration("Google publishes the Android SDK build tools for Linux on x86-64 only")

    def validate_build(self):
        # The same agreement sdkmanager asks to accept before it downloads anything
        if not self.conf.get("user.android-sdk:accept_license", default=False, check_type=bool):
            raise ConanInvalidConfiguration(
                "The Android SDK is downloaded from Google under the Android Software Development Kit License "
                "Agreement, https://developer.android.com/studio/terms. To accept it, pass "
                "-c:b user.android-sdk:accept_license=True")

    def _download(self, archive, folder):
        filename, sha1 = archive
        # Each archive has one top-level folder (e.g. android-15/ for build tools 35), which sdkmanager renames.
        get(self, self._url + filename, sha1=sha1, destination=os.path.join(self.build_folder, folder),
            strip_root=True, keep_permissions=True)

    def build(self):
        self._download(self._build_tools[str(self.settings.os)], self._build_tools_folder)
        self._download(self._platform_tools[str(self.settings.os)], "platform-tools")
        self._download(self._platform_archive, self._platform_folder)

    def package(self):
        for folder in ("build-tools", "platforms", "platform-tools"):
            copy(self, "*", src=os.path.join(self.build_folder, folder), dst=os.path.join(self.package_folder, folder))

    def package_info(self):
        self.cpp_info.includedirs = []
        self.cpp_info.libdirs = []
        # Puts adb and the build tools on the PATH of the build environment
        self.cpp_info.bindirs = ["platform-tools", self._build_tools_folder]
        self.buildenv_info.define_path("ANDROID_HOME", self.package_folder)
