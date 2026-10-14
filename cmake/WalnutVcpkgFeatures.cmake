# Selects the vcpkg.json features matching RENDERER and WINDOWING_SYSTEM, so the vcpkg toolchain
# installs the right dependencies. Has to be included before project(). Without vcpkg it has no effect.

if(RENDERER STREQUAL "OpenGL")
    list(APPEND VCPKG_MANIFEST_FEATURES "opengl")
elseif(RENDERER STREQUAL "Vulkan")
    list(APPEND VCPKG_MANIFEST_FEATURES "vulkan")
endif()

if(WINDOWING_SYSTEM STREQUAL "SDL")
    list(APPEND VCPKG_MANIFEST_FEATURES "sdl")
else()
    list(APPEND VCPKG_MANIFEST_FEATURES "glfw")
endif()
