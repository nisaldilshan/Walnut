add_library (walnut-graphics-webgpu STATIC
                    ImGuiBuildWGPU.cpp
                    WebGPUGraphics.cpp
                    WebGPUImage.cpp
                    ${imgui_INCLUDE_DIRS}/imgui_impl_wgpu.cpp
                )

if (WINDOWING_SYSTEM STREQUAL "SDL")
    set(WEBGPU_BACKEND_DAWN 1)
    FetchContent_Declare(
        sdl3webgpu
        GIT_REPOSITORY https://github.com/eliemichel/sdl3webgpu.git
        GIT_TAG        v1.0.0
        SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external_deps/sdl3webgpu-src
        BINARY_DIR     ${CMAKE_SOURCE_DIR}/external_deps/sdl3webgpu-build
        SUBBUILD_DIR   ${CMAKE_SOURCE_DIR}/external_deps/sdl3webgpu-subbuild
    )
    FetchContent_MakeAvailable(sdl3webgpu)

    target_sources(walnut-graphics-webgpu PRIVATE SDLWebGPURenderingBackend.cpp)
    target_link_libraries(walnut-graphics-webgpu PUBLIC sdl3webgpu)
else()
    if(WIN32)
        set(GLFW_BUILD_WIN32 1)
    elseif(APPLE)
        set(GLFW_BUILD_COCOA 1)
    else()
        set(GLFW_BUILD_X11 1) # or canbe GLFW_BUILD_WAYLAND
    endif()

    FetchContent_Declare(
        glfw3webgpu
        GIT_REPOSITORY https://github.com/eliemichel/glfw3webgpu.git
        GIT_TAG        v1.3.0-alpha
        SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external_deps/glfw3webgpu-src
        BINARY_DIR     ${CMAKE_SOURCE_DIR}/external_deps/glfw3webgpu-build
        SUBBUILD_DIR   ${CMAKE_SOURCE_DIR}/external_deps/glfw3webgpu-subbuild
    )
    FetchContent_MakeAvailable(glfw3webgpu)

    target_sources(walnut-graphics-webgpu PRIVATE GlfwWebGPURenderingBackend.cpp)
    target_link_libraries(walnut-graphics-webgpu PUBLIC glfw3webgpu glfw)
endif()


set_source_files_properties(${imgui_INCLUDE_DIRS}/imgui_impl_wgpu.cpp PROPERTIES COMPILE_DEFINITIONS "IMGUI_IMPL_WEBGPU_BACKEND_DAWN=1")

if(APPLE)
set_source_files_properties(${imgui_INCLUDE_DIRS}/imgui_impl_wgpu.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
else()           
endif()

target_include_directories(walnut-graphics-webgpu PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src) 
target_link_libraries(walnut-graphics-webgpu PUBLIC imgui::imgui glm::glm webgpu)
