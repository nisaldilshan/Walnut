cmake_minimum_required (VERSION 3.16.0)
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

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
else()
    message("-- CMAKE_BUILD_TYPE=" "${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "CMake Source Dir " + ${CMAKE_SOURCE_DIR})
message(STATUS "CMake Binary Dir " + ${CMAKE_BINARY_DIR})

set(CONAN_DISABLE_CHECK_COMPILER On)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

set(CMAKE_INSTALL_RPATH "@executable_path")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

option(USE_OPENGL_RENDERER "USE OPENGL RENDERER" OFF)

if (USE_OPENGL_RENDERER)
    find_package (OpenGL REQUIRED)
else()
    find_package (Vulkan REQUIRED)
endif()
#find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets REQUIRED)

### if QT version is 5
# set(CMAKE_AUTOMOC ON) 
# set(CMAKE_AUTORCC ON) 
# set(CMAKE_AUTOUIC ON)
### if QT version is 6
#qt_standard_project_setup()


#### Third Party Libs which are not already built ####
add_library(walnut-imgui SHARED
                Walnut/src/Walnut/ImGui/ImGuiBuild.cpp
                )
#### WALNUT GRAPHICS ###
if (USE_OPENGL_RENDERER)
    add_library (walnut-graphics-opengl STATIC
                    Walnut/src/Walnut/ImGui/ImGuiBuildOpenGL.cpp
                    Walnut/src/Walnut/GraphicsAPI/GlfwOpenGLRenderingBackend.cpp
                    Walnut/src/Walnut/GraphicsAPI/OpenGLImage.cpp   
                )

    target_link_libraries(walnut-graphics-opengl PUBLIC walnut-imgui ${CONAN_LIBS})
else()
    add_library (walnut-graphics-vulkan SHARED
                    Walnut/src/Walnut/ImGui/ImGuiBuildVulkan.cpp
                    Walnut/src/Walnut/GraphicsAPI/VulkanGraphics.cpp
                    Walnut/src/Walnut/GraphicsAPI/GlfwVulkanRenderingBackend.cpp
                    Walnut/src/Walnut/GraphicsAPI/VulkanImage.cpp
                )
    target_link_libraries(walnut-graphics-vulkan PUBLIC walnut-imgui ${CONAN_LIBS})
    if (NOT APPLE)
        target_link_libraries(walnut-graphics-vulkan PRIVATE Vulkan::Vulkan)
    endif()
endif()

#### ENGINE ###
add_library (walnut STATIC
                Walnut/src/Walnut/Application.cpp
                Walnut/src/Walnut/Image.cpp
                Walnut/src/Walnut/Random.cpp
                Walnut/src/Walnut/Input/Input.cpp
                Walnut/src/Walnut/RenderingBackend.cpp
            )

if (USE_OPENGL_RENDERER)
    add_compile_definitions(USE_OPENGL_RENDERER)
endif()

#target_compile_definitions(walnut PUBLIC VK_USE_PLATFORM_MACOS_MVK)

target_include_directories(walnut PRIVATE "Walnut/src")
target_include_directories(walnut PRIVATE ${CONAN_INCLUDE_DIRS})
if(MSVC)
    target_include_directories(walnut PUBLIC "C:\\VulkanSDK\\1.3.250.1\\Include")
    target_link_directories(walnut INTERFACE "C:\\VulkanSDK\\1.3.250.1\\Lib")
endif()
target_link_libraries(walnut PRIVATE ${CONAN_LIBS})
if (USE_OPENGL_RENDERER)
    target_link_libraries(walnut PRIVATE walnut-graphics-opengl)
else()
    target_link_libraries(walnut PRIVATE walnut-graphics-vulkan)
endif()
# if(MSVC)
#     target_link_libraries(walnut INTERFACE vulkan-1.lib)
# endif()

#set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")
#set_target_properties(walnut PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")
if(APPLE)
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
