# Finds Walnut's dependencies, which come either from Conan or from vcpkg. All differences between
# the two package managers are handled here; the rest of the build only uses what this file provides:
#
#   imgui::imgui, stb::stb          imgui and stb_image
#   WALNUT_WINDOWING_TARGET         glfw or SDL3::SDL3
#   WALNUT_VULKAN_PACKAGE           package providing the Vulkan loader: Vulkan, VulkanLoader or moltenvk
#   WALNUT_VULKAN_TARGET            Vulkan loader target
#   walnut_add_imgui_backend()      creates the imgui backend library for a renderer (see below)

set(CONAN_DISABLE_CHECK_COMPILER On)

#### IMGUI ###
find_package(imgui REQUIRED)

# Conan's imgui package ships the backend sources under res/ and Walnut compiles the ones it needs.
# vcpkg's imgui already has the backends built in, selected through the features in vcpkg.json.
if(DEFINED imgui_INCLUDE_DIRS AND EXISTS "${imgui_INCLUDE_DIRS}/../res/bindings")
    set(WALNUT_IMGUI_RES_DIR "${imgui_INCLUDE_DIRS}/../res")
endif()

#### STB ###
# Conan provides stb::stb, vcpkg provides FindStb.cmake which only sets Stb_INCLUDE_DIR.
find_package(stb CONFIG QUIET)
if(NOT TARGET stb::stb)
    find_package(Stb REQUIRED)
    add_library(stb::stb INTERFACE IMPORTED)
    target_include_directories(stb::stb INTERFACE ${Stb_INCLUDE_DIR})
endif()

#### WINDOWING SYSTEM ###
if(WINDOWING_SYSTEM STREQUAL "SDL")
    find_package(SDL3 REQUIRED)
    set(WALNUT_WINDOWING_TARGET SDL3::SDL3)
    set(WALNUT_IMGUI_PLATFORM_BINDING sdl3)
else()
    find_package(glfw3 REQUIRED)
    set(WALNUT_WINDOWING_TARGET glfw)
    set(WALNUT_IMGUI_PLATFORM_BINDING glfw)
endif()

#### RENDERER ###
if(RENDERER STREQUAL "OpenGL")
    find_package(glad REQUIRED)

elseif(RENDERER STREQUAL "Vulkan")
    if(VCPKG_TOOLCHAIN)
        # The same target vcpkg's imgui links against. On macOS, MoltenVK has to be installed
        # separately (e.g. brew install molten-vk) since vcpkg has no moltenvk port.
        set(WALNUT_VULKAN_PACKAGE Vulkan)
        set(WALNUT_VULKAN_TARGET Vulkan::Vulkan)
    elseif(APPLE)
        set(WALNUT_VULKAN_PACKAGE moltenvk)
        set(WALNUT_VULKAN_TARGET Vulkan::Loader)
    else()
        set(WALNUT_VULKAN_PACKAGE VulkanLoader)
        set(WALNUT_VULKAN_TARGET Vulkan::Loader)
        # Only used to print VK_LAYER_PATH during the build.
        find_package(vulkan-validationlayers REQUIRED)
    endif()
    find_package(${WALNUT_VULKAN_PACKAGE} REQUIRED)

elseif(RENDERER STREQUAL "WebGPU")
    if(NOT WALNUT_IMGUI_RES_DIR)
        message(FATAL_ERROR "The WebGPU renderer needs the imgui backend sources from Conan's imgui package; it is not supported with vcpkg.")
    endif()
    set(WEBGPU_BACKEND DAWN)
    FetchContent_Declare(
        WebGPU-distribution
        GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution.git
        GIT_TAG        v0.3.0-gamma
        QUIET
        SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external_deps/WebGPU-distribution-src
        BINARY_DIR     ${CMAKE_SOURCE_DIR}/external_deps/WebGPU-distribution-build
        SUBBUILD_DIR   ${CMAKE_SOURCE_DIR}/external_deps/WebGPU-distribution-subbuild
    )
    FetchContent_MakeAvailable(WebGPU-distribution)
endif()

# walnut_add_imgui_backend(<target> <renderer binding> [<libraries>...])
#
# Creates <target> providing imgui together with its <renderer binding> backend (e.g. vulkan,
# opengl3, wgpu) and the backend for the selected windowing system, linked to <libraries>.
# With Conan this compiles imgui from the package's sources; with vcpkg, imgui::imgui already
# contains the backends and <target> is an interface library.
function(walnut_add_imgui_backend target renderer_binding)
    if(WALNUT_IMGUI_RES_DIR)
        add_library(${target} STATIC
            ${WALNUT_IMGUI_RES_DIR}/src/imgui.cpp
            ${WALNUT_IMGUI_RES_DIR}/src/imgui_demo.cpp
            ${WALNUT_IMGUI_RES_DIR}/src/imgui_draw.cpp
            ${WALNUT_IMGUI_RES_DIR}/src/imgui_tables.cpp
            ${WALNUT_IMGUI_RES_DIR}/src/imgui_widgets.cpp
            ${WALNUT_IMGUI_RES_DIR}/bindings/imgui_impl_${renderer_binding}.cpp
            ${WALNUT_IMGUI_RES_DIR}/bindings/imgui_impl_${WALNUT_IMGUI_PLATFORM_BINDING}.cpp
        )
        target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${WALNUT_IMGUI_RES_DIR}/bindings>)
        set(scope PUBLIC)
    else()
        add_library(${target} INTERFACE)
        set(scope INTERFACE)
    endif()
    target_link_libraries(${target} ${scope} imgui::imgui ${WALNUT_WINDOWING_TARGET} ${ARGN})
endfunction()
