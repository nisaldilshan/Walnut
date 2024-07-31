add_library (walnut-graphics-vulkan STATIC
                    ImGuiBuildVulkan.cpp
                    VulkanGraphics.cpp
                    GlfwVulkanRenderingBackend.cpp
                    VulkanImage.cpp
                )

target_include_directories(walnut-graphics-vulkan PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src)  

target_link_libraries(walnut-graphics-vulkan PUBLIC imgui::imgui glfw Vulkan::Vulkan)