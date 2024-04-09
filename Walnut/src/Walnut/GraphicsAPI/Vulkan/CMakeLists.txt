add_library (walnut-graphics-vulkan STATIC
                    ImGuiBuildVulkan.cpp
                    VulkanGraphics.cpp
                    GlfwVulkanRenderingBackend.cpp
                    VulkanImage.cpp
                )

target_include_directories(walnut-graphics-vulkan PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src)  

target_link_libraries(walnut-graphics-vulkan PUBLIC ${CONAN_LIBS})
if (NOT APPLE)
    target_link_libraries(walnut-graphics-vulkan PRIVATE Vulkan::Vulkan)
endif()