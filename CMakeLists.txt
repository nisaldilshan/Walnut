cmake_minimum_required (VERSION 3.22.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS OFF)

# Has to run before project() so the vcpkg toolchain installs the right dependencies.
include(${CMAKE_CURRENT_LIST_DIR}/cmake/WalnutVcpkgFeatures.cmake)

project (Walnut VERSION 2.1.0)

include(FetchContent)
include(GNUInstallDirs)

#### OPTIONS ###
# Declared after project() so values set by the Conan toolchain take precedence.
set(RENDERER "WebGPU" CACHE STRING "Renderer to be used: OpenGL, Vulkan or WebGPU")
set(WINDOWING_SYSTEM "GLFW" CACHE STRING "Windowing system to be used: GLFW or SDL")
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

#### DEPENDENCIES ###
# Everything that differs between Conan and vcpkg is handled in this file.
include(cmake/WalnutDependencies.cmake)

#### PLATFORM SETTINGS ###
set(CMAKE_INSTALL_RPATH "@executable_path")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")
# The static graphics libraries end up in walnut, which is a shared library on Linux and macOS.
if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "iOS")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
    set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")
    add_compile_definitions(__iOS__)
endif()

#### WALNUT GRAPHICS ###
# Builds walnut-graphics-<renderer> and walnut-imgui-<renderer>.
add_compile_definitions(RENDERER_BACKEND=${WALNUT_RENDERER_BACKEND})
if (WINDOWING_SYSTEM STREQUAL "SDL")
    add_compile_definitions(USE_SDL)
endif()
add_subdirectory(Walnut/src/Walnut/GraphicsAPI/${RENDERER})

#### ENGINE ###
if(WIN32)
    add_library (walnut STATIC)
elseif(DEFINED BUILD_SHARED_LIBS)
    add_library (walnut) # follows BUILD_SHARED_LIBS, e.g. as set by the vcpkg triplet
else()
    add_library (walnut SHARED)
endif()
add_library(Walnut::walnut ALIAS walnut)

target_sources(walnut PRIVATE
                Walnut/src/Walnut/Image.cpp
                Walnut/src/Walnut/Random.cpp
                Walnut/src/Walnut/RenderingBackend.cpp
                Walnut/src/Walnut/Application.cpp
)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut PRIVATE Walnut/src/Walnut/ApplicationSDL.cpp
                                Walnut/src/Walnut/Input/SDLInput.cpp)
    target_link_libraries(walnut PRIVATE SDL3::SDL3)
    target_compile_definitions(walnut INTERFACE USE_SDL)
else()
    target_sources(walnut PRIVATE Walnut/src/Walnut/ApplicationGLFW.cpp
                                Walnut/src/Walnut/Input/GLFWInput.cpp)
endif()

target_compile_definitions(walnut PRIVATE Walnut_EXPORTS)
target_include_directories(walnut PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Walnut/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(walnut PRIVATE $<BUILD_INTERFACE:stb::stb> PUBLIC imgui::imgui)

if (RENDERER STREQUAL "Vulkan")
    target_link_libraries(walnut PRIVATE walnut-graphics-vulkan)
else()
    target_link_libraries(walnut PUBLIC walnut-graphics-${WALNUT_RENDERER_NAME})
endif()
# Also passed on to consumers since the public headers depend on it.
target_compile_definitions(walnut INTERFACE RENDERER_BACKEND=${WALNUT_RENDERER_BACKEND})

#### PLATFORM SPECIFICS ###
string(COMPARE EQUAL "${CMAKE_SYSTEM_NAME}" "Emscripten" IS_EMSCRIPTEN) # also used by WalnutApp
if (IS_EMSCRIPTEN)
    message("This is an Emscripten Build")
    set_target_properties(walnut-graphics-opengl PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3")
    set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3")
    #set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")
endif()

if(WALNUT_VULKAN_PACKAGE STREQUAL "moltenvk")
    set(MOLTENVK_DIR
        $<$<CONFIG:Debug>:${moltenvk_LIB_DIRS_DEBUG}>$<$<CONFIG:RelWithDebInfo>:${moltenvk_LIB_DIRS_RELWITHDEBINFO}>)
    message(STATUS "MOLTENVK_DIR" + ${MOLTENVK_DIR})
    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND ln -sf ${MOLTENVK_DIR}/libMoltenVK.dylib libMoltenVK.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                        COMMENT "Generating symbolic link for libMoltenVK!!!")

    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND ln -sf ${MOLTENVK_DIR}/libMoltenVK.dylib libvulkan.1.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                        COMMENT "Generating symbolic link for libvulkan.1!!!")
endif()

#### EXAMPLE ###
if(WALNUT_BUILD_EXAMPLES)
    add_subdirectory(WalnutApp)
endif()

#### INSTALL ###
if(WALNUT_INSTALL)
    include(cmake/WalnutInstall.cmake)
endif()
