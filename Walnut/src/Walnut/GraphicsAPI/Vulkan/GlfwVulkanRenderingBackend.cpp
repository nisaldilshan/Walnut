#include "VulkanRenderingBackend.h"
#include <iostream>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "VulkanGraphics.h"

namespace Walnut
{

    void VulkanRenderingBackend::Init(GLFWwindow *windowHandle)
    {
        if (!glfwVulkanSupported())
        {
            std::cerr << "GLFW: Vulkan not supported!\n";
            return;
        }

        m_extensions = glfwGetRequiredInstanceExtensions(&m_extensions_count);
        m_windowHandle = windowHandle;

        // Setup Vulkan
        GraphicsAPI::Vulkan::SetupVulkan(m_extensions, m_extensions_count);
        // Create Window Surface
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
    }

    void VulkanRenderingBackend::Cleanup()
    {
        GraphicsAPI::Vulkan::CleanupVulkanWindow();
        GraphicsAPI::Vulkan::CleanupVulkan();
    }

}