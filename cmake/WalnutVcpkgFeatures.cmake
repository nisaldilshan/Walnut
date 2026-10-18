# Selects the vcpkg.json features matching RENDERER and WINDOWING_SYSTEM, so the vcpkg toolchain
# installs the right dependencies. Has to be included before project(). Without vcpkg it has no effect.
#
# The options themselves are declared after project() (see WalnutOptions.cmake), so on the first
# configure they are only set here if they were passed on the command line.

set(_walnut_renderer "${WALNUT_DEFAULT_RENDERER}")
if(DEFINED RENDERER)
    set(_walnut_renderer "${RENDERER}")
endif()
set(_walnut_windowing_system "${WALNUT_DEFAULT_WINDOWING_SYSTEM}")
if(DEFINED WINDOWING_SYSTEM)
    set(_walnut_windowing_system "${WINDOWING_SYSTEM}")
endif()

if(_walnut_renderer STREQUAL "OpenGL")
    list(APPEND VCPKG_MANIFEST_FEATURES "opengl")
elseif(_walnut_renderer STREQUAL "Vulkan")
    list(APPEND VCPKG_MANIFEST_FEATURES "vulkan")
endif()

if(_walnut_windowing_system STREQUAL "SDL")
    list(APPEND VCPKG_MANIFEST_FEATURES "sdl")
else()
    list(APPEND VCPKG_MANIFEST_FEATURES "glfw")
endif()
