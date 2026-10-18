# Finds Walnut's dependencies when they come from vcpkg (vcpkg.json).
# Provides the same as WalnutConan.cmake, see the DEPENDENCIES section of the root CMakeLists.txt.

# Walnut builds imgui's WebGPU backend from Conan's imgui sources, against the Dawn build it downloads
# itself. vcpkg's imgui only works with vcpkg's own (newer) Dawn.
if(RENDERER STREQUAL "WebGPU")
    message(FATAL_ERROR "The WebGPU renderer is not supported with vcpkg, use RENDERER=Vulkan or OpenGL, or build with Conan.")
endif()

#### IMGUI ###
# vcpkg's imgui already has the backends built in, selected through the features in vcpkg.json.
find_package(imgui REQUIRED)

#### STB ###
# vcpkg provides FindStb.cmake, which only sets Stb_INCLUDE_DIR.
find_package(Stb REQUIRED)
if(NOT TARGET stb::stb)
    add_library(stb::stb INTERFACE IMPORTED)
    target_include_directories(stb::stb INTERFACE ${Stb_INCLUDE_DIR})
endif()

#### WINDOWING SYSTEM ###
if(WINDOWING_SYSTEM STREQUAL "SDL")
    find_package(SDL3 REQUIRED)
    set(WALNUT_WINDOWING_TARGET SDL3::SDL3)
else()
    find_package(glfw3 REQUIRED)
    set(WALNUT_WINDOWING_TARGET glfw)
endif()

#### RENDERER ###
if(RENDERER STREQUAL "OpenGL")
    find_package(glad REQUIRED)

elseif(RENDERER STREQUAL "Vulkan")
    # The same target vcpkg's imgui links against. On macOS, MoltenVK has to be installed
    # separately (e.g. brew install molten-vk) since vcpkg has no moltenvk port.
    set(WALNUT_VULKAN_PACKAGE Vulkan)
    set(WALNUT_VULKAN_TARGET Vulkan::Vulkan)
    find_package(Vulkan REQUIRED)
endif()

# walnut_add_imgui_backend(<target> <renderer binding> [<libraries>...])
#
# Creates the interface library <target> linking imgui, the selected windowing system and <libraries>.
# imgui::imgui already contains the backends, so <renderer binding> is not used here.
function(walnut_add_imgui_backend target renderer_binding)
    add_library(${target} INTERFACE)
    target_link_libraries(${target} INTERFACE imgui::imgui ${WALNUT_WINDOWING_TARGET} ${ARGN})
endfunction()
