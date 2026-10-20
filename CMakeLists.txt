cmake_minimum_required (VERSION 3.22.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS OFF)

# Defaults for the RENDERER and WINDOWING_SYSTEM options.
set(WALNUT_DEFAULT_RENDERER Vulkan)
set(WALNUT_DEFAULT_WINDOWING_SYSTEM GLFW)

# Has to run before project() so the vcpkg toolchain installs the right dependencies.
include(${CMAKE_CURRENT_LIST_DIR}/cmake/WalnutVcpkgFeatures.cmake)

project (Walnut VERSION 2.2.0)

include(FetchContent)
include(GNUInstallDirs)

#### OPTIONS ###
# RENDERER, WINDOWING_SYSTEM, WALNUT_BUILD_EXAMPLES and WALNUT_INSTALL.
include(cmake/WalnutOptions.cmake)

#### DEPENDENCIES ###
# Dependencies come from vcpkg or Conan. Both files provide the same to the rest of the build:
#   imgui::imgui, stb::stb          imgui and stb_image
#   WALNUT_WINDOWING_TARGET         glfw or SDL3::SDL3
#   WALNUT_VULKAN_PACKAGE           package providing the Vulkan loader: Vulkan, VulkanLoader or moltenvk
#   WALNUT_VULKAN_TARGET            Vulkan loader target
#   walnut_add_imgui_backend()      creates the imgui backend library for a renderer
if(VCPKG_TOOLCHAIN)
    include(cmake/WalnutVcpkg.cmake)
else()
    include(cmake/WalnutConan.cmake)
endif()

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
