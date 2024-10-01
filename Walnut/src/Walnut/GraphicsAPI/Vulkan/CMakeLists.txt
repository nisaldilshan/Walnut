add_library (walnut-graphics-vulkan STATIC
                    ImGuiBuildVulkan.cpp
                    VulkanGraphics.cpp
                    VulkanImage.cpp
                )

target_include_directories(walnut-graphics-vulkan PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src)  

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut-graphics-vulkan PRIVATE SdlVulkanRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-vulkan PRIVATE SDL2::SDL2)
else()
    target_sources(walnut-graphics-vulkan PRIVATE GlfwVulkanRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-vulkan PUBLIC glfw)
endif()

if(APPLE)
    target_link_libraries(walnut-graphics-vulkan PUBLIC imgui::imgui moltenvk::moltenvk)
else()
    target_link_libraries(walnut-graphics-vulkan PUBLIC imgui::imgui Vulkan::Loader vulkan-validationlayers::vulkan-validationlayers)
endif()


