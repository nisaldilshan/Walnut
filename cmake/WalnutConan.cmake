# Finds Walnut's dependencies when they come from Conan (conanfile.py).
# Provides the same as WalnutVcpkg.cmake, see the DEPENDENCIES section of the root CMakeLists.txt.

set(CONAN_DISABLE_CHECK_COMPILER On)

#### IMGUI ###
find_package(imgui REQUIRED)

# Conan's imgui package ships the backend sources under res/ and Walnut compiles the ones it needs.
set(WALNUT_IMGUI_RES_DIR "${imgui_INCLUDE_DIRS}/../res")
if(NOT EXISTS "${WALNUT_IMGUI_RES_DIR}/bindings")
    message(FATAL_ERROR "Conan's imgui package has no backend sources in ${WALNUT_IMGUI_RES_DIR}/bindings")
endif()

#### STB ###
find_package(stb REQUIRED)

#### WINDOWING SYSTEM ###
if(WINDOWING_SYSTEM STREQUAL "SDL")
    find_package(SDL3 REQUIRED)
    set(WALNUT_WINDOWING_TARGET SDL3::SDL3)
    set(WALNUT_IMGUI_PLATFORM_BINDING sdl3)
elseif(EMSCRIPTEN)
    # Emscripten ships its own GLFW, so there is no package to find.
    add_library(glfw INTERFACE IMPORTED)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
    set(WALNUT_WINDOWING_TARGET glfw)
    set(WALNUT_IMGUI_PLATFORM_BINDING glfw)
else()
    find_package(glfw3 REQUIRED)
    set(WALNUT_WINDOWING_TARGET glfw)
    set(WALNUT_IMGUI_PLATFORM_BINDING glfw)
endif()

#### RENDERER ###
if(RENDERER STREQUAL "OpenGL")
    if(NOT EMSCRIPTEN) # WebGL needs no loader
        find_package(glad REQUIRED)
    endif()

elseif(RENDERER STREQUAL "Vulkan")
    if(APPLE)
        set(WALNUT_VULKAN_PACKAGE moltenvk)
    else()
        set(WALNUT_VULKAN_PACKAGE VulkanLoader)
        # Only used to print VK_LAYER_PATH during the build.
        find_package(vulkan-validationlayers REQUIRED)
    endif()
    set(WALNUT_VULKAN_TARGET Vulkan::Loader)
    find_package(${WALNUT_VULKAN_PACKAGE} REQUIRED)

elseif(RENDERER STREQUAL "WebGPU")
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
# Creates the static library <target>: imgui compiled from the package's sources, together with its
# <renderer binding> backend (e.g. vulkan, opengl3, wgpu) and the backend for the selected windowing
# system, linked to <libraries>.
function(walnut_add_imgui_backend target renderer_binding)
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
    target_link_libraries(${target} PUBLIC imgui::imgui ${WALNUT_WINDOWING_TARGET} ${ARGN})
endfunction()
