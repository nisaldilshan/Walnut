walnut_add_imgui_backend(walnut-imgui-vulkan vulkan ${WALNUT_VULKAN_TARGET})

###################################################################################################################################

add_library (walnut-graphics-vulkan STATIC
                    VulkanGraphics.cpp
                    VulkanImage.cpp
                    VulkanImageRenderPipeline.cpp
                    VulkanRenderingBackend.cpp
                )

target_include_directories(walnut-graphics-vulkan PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Walnut/src>)
target_link_libraries(walnut-graphics-vulkan PUBLIC imgui::imgui walnut-imgui-vulkan)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut-graphics-vulkan PRIVATE SdlVulkanRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-vulkan PRIVATE SDL3::SDL3)
else()
    target_sources(walnut-graphics-vulkan PRIVATE GlfwVulkanRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-vulkan PUBLIC glfw)
endif()

if(WALNUT_VULKAN_PACKAGE STREQUAL "moltenvk")
    if(CMAKE_SYSTEM_NAME MATCHES "iOS")
        target_include_directories(walnut-graphics-vulkan PUBLIC /Users/nisal/Downloads/MoltenVK/MoltenVK/include)
        target_link_libraries(walnut-graphics-vulkan PUBLIC
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
        target_link_libraries(walnut-graphics-vulkan PUBLIC moltenvk::moltenvk)
    endif()
else()
    target_link_libraries(walnut-graphics-vulkan PUBLIC ${WALNUT_VULKAN_TARGET})
endif()

if(vulkan-validationlayers_FOUND)
    # Create a custom target that echoes the path during the build step
    add_custom_target(PrintVulkanLayerPath ALL
        COMMAND ${CMAKE_COMMAND} -E echo "VK_LAYER_PATH for validation layers is: $<IF:$<CONFIG:Debug>,${vulkan-validationlayers_PACKAGE_FOLDER_DEBUG},Disabled for non debug builds>"
    )
endif()
