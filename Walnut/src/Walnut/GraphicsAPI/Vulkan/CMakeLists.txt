add_library (walnut-graphics-vulkan SHARED
                    ../../ImGui/ImGuiBuildVulkan.cpp
                    VulkanGraphics.cpp
                    GlfwVulkanRenderingBackend.cpp
                    VulkanImage.cpp
                )
target_link_libraries(walnut-graphics-vulkan PUBLIC walnut-imgui ${CONAN_LIBS})
if (NOT APPLE)
    target_link_libraries(walnut-graphics-vulkan PRIVATE Vulkan::Vulkan)
endif()