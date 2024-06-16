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
message("-- CMAKE_BUILD_TYPE=" "${CMAKE_BUILD_TYPE}")

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Setting _DEBUG")
    add_compile_definitions(_DEBUG)
endif()



message(STATUS "CMake Source Dir " + ${CMAKE_SOURCE_DIR})
message(STATUS "CMake Binary Dir " + ${CMAKE_BINARY_DIR})

set(CONAN_DISABLE_CHECK_COMPILER On)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

set(CMAKE_INSTALL_RPATH "@executable_path")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")

option(RENDERER "Renderer to be used" "WebGPU")

#### WALNUT GRAPHICS ###
if (RENDERER STREQUAL "OpenGL")
    add_subdirectory(Walnut/src/Walnut/GraphicsAPI/OpenGL)
elseif(RENDERER STREQUAL "Vulkan")
    find_package (Vulkan REQUIRED)
    add_subdirectory(Walnut/src/Walnut/GraphicsAPI/Vulkan)
elseif(RENDERER STREQUAL "WebGPU")
    add_subdirectory(vendor/glfw3webgpu)
    add_subdirectory(Walnut/src/Walnut/GraphicsAPI/WebGPU)
else()
    message(STATUS "NO RENDERING BACKEND")
endif()

#### ENGINE ###
if(MSVC)
add_library (walnut STATIC
                Walnut/src/Walnut/Application.cpp
                Walnut/src/Walnut/Image.cpp
                Walnut/src/Walnut/Random.cpp
                Walnut/src/Walnut/Input/Input.cpp
                Walnut/src/Walnut/RenderingBackend.cpp
            )
else()
add_library (walnut SHARED
                Walnut/src/Walnut/Application.cpp
                Walnut/src/Walnut/Image.cpp
                Walnut/src/Walnut/Random.cpp
                Walnut/src/Walnut/Input/Input.cpp
                Walnut/src/Walnut/RenderingBackend.cpp
            )
endif()
target_compile_definitions(walnut PRIVATE Walnut_EXPORTS)
target_include_directories(walnut PRIVATE "Walnut/src")
target_include_directories(walnut PRIVATE ${CONAN_INCLUDE_DIRS})
# if(MSVC)
#     target_include_directories(walnut PUBLIC "C:\\VulkanSDK\\1.3.250.1\\Include")
#     target_link_directories(walnut INTERFACE "C:\\VulkanSDK\\1.3.250.1\\Lib")
# endif()
#target_link_libraries(walnut PRIVATE ${CONAN_LIBS})
if (RENDERER STREQUAL "OpenGL")
    target_link_libraries(walnut PRIVATE walnut-graphics-opengl)
    add_compile_definitions(RENDERER_BACKEND=1)
elseif (RENDERER STREQUAL "Vulkan")
    target_link_libraries(walnut PRIVATE walnut-graphics-vulkan)
    add_compile_definitions(RENDERER_BACKEND=2)
else()
    target_link_libraries(walnut PRIVATE walnut-graphics-webgpu)
    add_compile_definitions(RENDERER_BACKEND=3)
endif()
# if(MSVC)
#     target_link_libraries(walnut INTERFACE vulkan-1.lib)
# endif()
string(COMPARE EQUAL "${CMAKE_SYSTEM_NAME}" "Emscripten" IS_EMSCRIPTEN)
if (IS_EMSCRIPTEN)
    message("This is an Emscripten Build")
    set_target_properties(walnut-graphics-opengl PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3")
    set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3")
    #set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")
endif()
if(APPLE AND RENDERER STREQUAL "Vulkan")
    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND rm -f bin/libMoltenVK.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
                        COMMENT "Removing symbolic link libMoltenVK!!!")
    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND rm -f bin/libvulkan.1.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
                        COMMENT "Removing symbolic link libvulkan.1!!!")

    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND ln -s ${CONAN_LIB_DIRS_MOLTENVK}/libMoltenVK.dylib bin/libMoltenVK.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
                        COMMENT "Generating symbolic link for libMoltenVK!!!")

    ADD_CUSTOM_COMMAND(TARGET walnut POST_BUILD
                        COMMAND ln -s ${CONAN_LIB_DIRS_MOLTENVK}/libMoltenVK.dylib bin/libvulkan.1.dylib
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
                        COMMENT "Generating symbolic link for libvulkan.1!!!")
endif()


add_subdirectory(WalnutApp)
