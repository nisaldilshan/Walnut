add_library (walnut-graphics-webgpu SHARED
                    ../../ImGui/ImGuiBuildWGPU.cpp
                    WebGPUGraphics.cpp
                    GlfwWebGPURenderingBackend.cpp
                    WebGPUImage.cpp
                )
target_link_libraries(walnut-graphics-webgpu PUBLIC walnut-imgui glfw3webgpu ${CONAN_LIB_DIRS_WEBGPU}/libwebgpu_dawn.dylib)
target_link_libraries(walnut-graphics-webgpu PUBLIC
    "-framework Metal"
    "-framework MetalKit"
    "-framework AppKit"
    "-framework Foundation"
    "-framework QuartzCore"
    "-framework IOSurface"
)