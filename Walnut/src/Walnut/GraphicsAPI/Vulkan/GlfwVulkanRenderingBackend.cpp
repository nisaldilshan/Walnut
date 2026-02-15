#include "VulkanRenderingBackend.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanGraphics.h"
#include <imgui_impl_glfw.h>
#include <iostream>

namespace Walnut
{
    void VulkanRenderingBackend::Init(GLFWwindow *windowHandle)
    {
        if (!glfwVulkanSupported())
        {
            std::cerr << "GLFW: Vulkan not supported!\n";
            return;
        }
        uint32_t extensionsCount = 0;
	    const char* const* glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

        ImVector<const char*> extensions;
        for (uint32_t n = 0; n < extensionsCount; n++) {
            // if (std::string(glfwExtensions[n]) == "VK_KHR_portability_enumeration") { 
            //     // TODO: somehow vkCreateInstance function fails when this extension is present
            //     std::cout << "   GLFW: Extension - " << glfwExtensions[n] << " skipping" << std::endl;
            //     continue;
            // }
            // else 
            {
                std::cout << "   GLFW: Extension - " << glfwExtensions[n] << std::endl;
            }
            extensions.push_back(glfwExtensions[n]);
        }
        m_windowHandle = windowHandle;

        // Setup Vulkan
        GraphicsAPI::Vulkan::SetupVulkan(extensions);
        // Create Window Surface
        VkResult err = glfwCreateWindowSurface(GraphicsAPI::Vulkan::GetInstance(), 
                                                windowHandle, 
                                                GraphicsAPI::Vulkan::GetAllocator(), 
                                                GraphicsAPI::Vulkan::GetSurface());
        GraphicsAPI::Vulkan::check_vk_result(err);
    }

    void VulkanRenderingBackend::SetupWindow(int width, int height)
    {
        GraphicsAPI::Vulkan::SetupVulkanWindow(width, height);
    }

    bool VulkanRenderingBackend::NeedToResizeWindow()
    {
        return GraphicsAPI::Vulkan::NeedSwapChainRebuild();
    }

    void VulkanRenderingBackend::ResizeWindow(int width, int height)
    {
        GraphicsAPI::Vulkan::ResizeVulkanWindow(width, height);
        GraphicsAPI::Vulkan::SetSwapChainRebuildStatus(false);
    }

    void VulkanRenderingBackend::ConfigureImGui()
    {
        ImGui_ImplGlfw_InitForVulkan(m_windowHandle, true);
        GraphicsAPI::Vulkan::ConfigureRendererBackend();
    }

    void VulkanRenderingBackend::StartImGuiFrame()
    {
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    void VulkanRenderingBackend::UploadFonts()
    {
        GraphicsAPI::Vulkan::UploadFonts();
    }

    void VulkanRenderingBackend::FrameRender(void* draw_data)
    {
        GraphicsAPI::Vulkan::FrameRender(draw_data);
    }

    void VulkanRenderingBackend::FramePresent()
    {
        GraphicsAPI::Vulkan::FramePresent();
    }

    GLFWwindow *VulkanRenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

    void VulkanRenderingBackend::Shutdown()
    {
        GraphicsAPI::Vulkan::GraphicsDeviceWaitIdle();
        GraphicsAPI::Vulkan::FreeGraphicsResources();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void VulkanRenderingBackend::Cleanup()
    {
        GraphicsAPI::Vulkan::CleanupVulkanWindow();
        GraphicsAPI::Vulkan::CleanupVulkan();
    }

}