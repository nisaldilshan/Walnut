#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from conans import ConanFile, CMake, tools


class IMGUIConan(ConanFile):
    name = "imgui"
    version = "latest"
    url = "https://github.com/bincrafters/conan-imgui"
    homepage = "https://github.com/ocornut/imgui"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    author = "Bincrafters <bincrafters@gmail.com>"
    topics = ("conan", "imgui", "gui", "graphical")
    license = "MIT"
    exports = ["LICENSE.md"]
    exports_sources = ["CMakeLists.txt", ]
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    _source_subfolder = "source_subfolder"

    def source(self):
        git = tools.Git(folder=self._source_subfolder)
        git.clone("https://github.com/ocornut/imgui.git", "docking")


    def package(self):
        self.copy(pattern="LICENSE.txt", dst="licenses", src=self._source_subfolder)
        self.copy(pattern="backends/imgui_impl_*", dst="include", src=self._source_subfolder, keep_path=False)
        self.copy(pattern="imconfig.h", dst="include", src=self._source_subfolder, keep_path=False)
        self.copy(pattern="imgui.h", dst="include", src=self._source_subfolder, keep_path=False)
        self.copy(pattern="imgui.cpp", dst="include", src=self._source_subfolder, keep_path=False)
        self.copy(pattern="imgui_*", dst="include", src=self._source_subfolder, keep_path=False)
        self.copy(pattern="imgui_*", dst="include", src=self._source_subfolder, keep_path=False)
        self.copy(pattern="imstb_*", dst="include", src=self._source_subfolder, keep_path=False)