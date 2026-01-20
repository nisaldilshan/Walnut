add_library (walnut-graphics-vulkan STATIC
                    ImGuiBuildVulkan.cpp
                    VulkanGraphics.cpp
                    VulkanImage.cpp
                )

target_include_directories(walnut-graphics-vulkan PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src)  

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut-graphics-vulkan PRIVATE SdlVulkanRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-vulkan PRIVATE SDL3::SDL3)
else()
    target_sources(walnut-graphics-vulkan PRIVATE GlfwVulkanRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-vulkan PUBLIC glfw)
endif()

if(APPLE)
    if(CMAKE_SYSTEM_NAME MATCHES "iOS")
        target_include_directories(walnut-graphics-vulkan PUBLIC /Users/nisal/Downloads/MoltenVK/MoltenVK/include)
        target_link_libraries(walnut-graphics-vulkan PUBLIC 
                                imgui::imgui 
                                /Users/nisal/Downloads/MoltenVK/MoltenVK/static/MoltenVK.xcframework/ios-arm64_x86_64-simulator/libMoltenVK.a
                                "-framework Metal"
                                "-framework MetalKit"
                                "-framework Foundation"
                                "-framework QuartzCore"
                                "-framework IOSurface"
                                "-framework CoreVideo"
                                "-framework IOKit"
                            )

    else()
        target_link_libraries(walnut-graphics-vulkan PUBLIC imgui::imgui moltenvk::moltenvk)
    endif()
else()
    target_link_libraries(walnut-graphics-vulkan PUBLIC imgui::imgui Vulkan::Loader) # vulkan-validationlayers::vulkan-validationlayers
endif()


