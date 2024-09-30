cmake_minimum_required (VERSION 3.22.0)
set(CMAKE_CXX_STANDARD 17)
message(STATUS "CMAKE_CXX_STANDARD " + ${CMAKE_CXX_STANDARD})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "CMAKE_CXX_STANDARD_REQUIRED " + ${CMAKE_CXX_STANDARD_REQUIRED})
set(CXX_EXTENSIONS OFF)
message(STATUS "CXX_EXTENSIONS " + ${CXX_EXTENSIONS})

project (Walnut)

message(STATUS "CMAKE_SYSTEM " + ${CMAKE_SYSTEM})
message(STATUS "CMAKE_SYSTEM_NAME " + ${CMAKE_SYSTEM_NAME})
message(STATUS "CMAKE_SYSTEM_VERSION " + ${CMAKE_SYSTEM_VERSION})
message(STATUS "CMAKE_SYSTEM_PROCESSOR " + ${CMAKE_SYSTEM_PROCESSOR})

message(STATUS "CMAKE_HOST_SYSTEM" + ${CMAKE_HOST_SYSTEM})

message(STATUS "CMAKE_CXX_COMPILER " + ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_COMPILER_ID " + ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER_VERSION " + ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_CXX_COMPILER_LOADED " + ${CMAKE_CXX_COMPILER_LOADED})
message(STATUS "CMAKE_BUILD_TYPE " + ${CMAKE_BUILD_TYPE})

message(STATUS "CMake Source Dir " + ${CMAKE_SOURCE_DIR})
message(STATUS "CMake Binary Dir " + ${CMAKE_BINARY_DIR})

set(CONAN_DISABLE_CHECK_COMPILER On)
find_package(imgui REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)

set(CMAKE_INSTALL_RPATH "@executable_path")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")

option(RENDERER "Renderer to be used" "WebGPU")
option(WINDOWING_SYSTEM "Windowing system to be used" "GLFW")
message(STATUS "RENDERER" + ${RENDERER})
message(STATUS "WINDOWING_SYSTEM" + ${WINDOWING_SYSTEM})

if (WINDOWING_SYSTEM STREQUAL "SDL")
    find_package(SDL2 REQUIRED)
    add_compile_definitions(USE_SDL)
else()
    find_package(glfw3 REQUIRED)
endif()

#### WALNUT GRAPHICS ###
if (RENDERER STREQUAL "OpenGL")
    find_package(glad REQUIRED)
    add_subdirectory(Walnut/src/Walnut/GraphicsAPI/OpenGL)
elseif(RENDERER STREQUAL "Vulkan")
    if(APPLE)
        find_package(moltenvk REQUIRED)
    else()
        find_package (VulkanLoader REQUIRED)
        find_package (vulkan-validationlayers REQUIRED)
    endif()
    add_subdirectory(Walnut/src/Walnut/GraphicsAPI/Vulkan)
elseif(RENDERER STREQUAL "WebGPU")
    find_package(WebGPU REQUIRED)
    add_subdirectory(vendor/glfw3webgpu)
    add_subdirectory(Walnut/src/Walnut/GraphicsAPI/WebGPU)
else()
    message(STATUS "NO RENDERING BACKEND")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "iOS")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")
endif()

#### ENGINE ###
if(WIN32)
add_library (walnut STATIC)
else()
add_library (walnut SHARED)
endif()

target_sources(walnut PRIVATE 
                Walnut/src/Walnut/Image.cpp
                Walnut/src/Walnut/Random.cpp
                Walnut/src/Walnut/Input/Input.cpp
                Walnut/src/Walnut/RenderingBackend.cpp
)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut PRIVATE Walnut/src/Walnut/ApplicationSDL.cpp)
else()
    target_sources(walnut PRIVATE Walnut/src/Walnut/Application.cpp)
endif()

target_compile_definitions(walnut PRIVATE Walnut_EXPORTS)
target_include_directories(walnut PRIVATE "Walnut/src")
target_link_libraries(walnut PUBLIC stb::stb glm::glm imgui::imgui)

if (RENDERER STREQUAL "OpenGL")
    target_link_libraries(walnut PUBLIC walnut-graphics-opengl)
    add_compile_definitions(RENDERER_BACKEND=1)
elseif (RENDERER STREQUAL "Vulkan")
    target_link_libraries(walnut PRIVATE walnut-graphics-vulkan)
    add_compile_definitions(RENDERER_BACKEND=2)
else()
    target_link_libraries(walnut PUBLIC walnut-graphics-webgpu)
    add_compile_definitions(RENDERER_BACKEND=3)
endif()

string(COMPARE EQUAL "${CMAKE_SYSTEM_NAME}" "Emscripten" IS_EMSCRIPTEN)
if (IS_EMSCRIPTEN)
    message("This is an Emscripten Build")
    set_target_properties(walnut-graphics-opengl PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3")
    set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3")
    #set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")
endif()
if(APPLE AND RENDERER STREQUAL "Vulkan")
    message(STATUS "moltenvk_LIB_DIRS_DEBUG" + ${moltenvk_LIB_DIRS_DEBUG})

    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND ln -sf ${moltenvk_LIB_DIRS_DEBUG}/libMoltenVK.dylib libMoltenVK.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
                        COMMENT "Generating symbolic link for libMoltenVK!!!")

    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND ln -sf ${moltenvk_LIB_DIRS_DEBUG}/libMoltenVK.dylib libvulkan.1.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
                        COMMENT "Generating symbolic link for libvulkan.1!!!")
endif()


# add_library(walnut-combined INTERFACE "")
# set_target_properties(walnut-combined PROPERTIES LINKER_LANGUAGE CXX)
# target_link_libraries(walnut-combined INTERFACE 
#   "$<LINK_LIBRARY:WHOLE_ARCHIVE,walnut>"
# )


add_subdirectory(WalnutApp)
