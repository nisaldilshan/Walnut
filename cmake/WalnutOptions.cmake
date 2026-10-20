# Declares and checks Walnut's build options. Has to be included after project(): the Conan toolchain
# sets RENDERER and WINDOWING_SYSTEM as cache variables during project(), and those take precedence
# over the defaults declared here.

set(RENDERER "${WALNUT_DEFAULT_RENDERER}" CACHE STRING "Renderer to be used: OpenGL, Vulkan or WebGPU")
set(WINDOWING_SYSTEM "${WALNUT_DEFAULT_WINDOWING_SYSTEM}" CACHE STRING "Windowing system to be used: GLFW or SDL")
option(WALNUT_BUILD_EXAMPLES "Build the WalnutApp example" ${PROJECT_IS_TOP_LEVEL})
option(WALNUT_INSTALL "Generate install rules and the Walnut CMake package" ${PROJECT_IS_TOP_LEVEL})

# The position in this list + 1 is the RENDERER_BACKEND value the sources check for.
set(WALNUT_RENDERERS OpenGL Vulkan WebGPU)
set_property(CACHE RENDERER PROPERTY STRINGS ${WALNUT_RENDERERS})
set_property(CACHE WINDOWING_SYSTEM PROPERTY STRINGS GLFW SDL)

list(FIND WALNUT_RENDERERS "${RENDERER}" WALNUT_RENDERER_INDEX)
if(WALNUT_RENDERER_INDEX EQUAL -1)
    message(FATAL_ERROR "Unknown RENDERER '${RENDERER}', expected one of: ${WALNUT_RENDERERS}")
endif()
if(NOT WINDOWING_SYSTEM MATCHES "^(GLFW|SDL)$")
    message(FATAL_ERROR "Unknown WINDOWING_SYSTEM '${WINDOWING_SYSTEM}', expected GLFW or SDL")
endif()
math(EXPR WALNUT_RENDERER_BACKEND "${WALNUT_RENDERER_INDEX} + 1")
string(TOLOWER "${RENDERER}" WALNUT_RENDERER_NAME)

foreach(var RENDERER WINDOWING_SYSTEM
            CMAKE_SYSTEM CMAKE_SYSTEM_NAME CMAKE_SYSTEM_VERSION CMAKE_SYSTEM_PROCESSOR CMAKE_HOST_SYSTEM
            CMAKE_CXX_COMPILER CMAKE_CXX_COMPILER_ID CMAKE_CXX_COMPILER_VERSION CMAKE_BUILD_TYPE
            CMAKE_SOURCE_DIR CMAKE_BINARY_DIR)
    message(STATUS "${var}: ${${var}}")
endforeach()
