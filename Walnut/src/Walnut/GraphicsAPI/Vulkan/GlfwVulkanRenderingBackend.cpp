#include "GlfwVulkanRenderingBackend.h"
#include <iostream>

#include "VulkanGraphics.h"

namespace Walnut
{

    void GlfwVulkanRenderingBackend::Init(GLFWwindow *windowHandle)
    {
        if (!glfwVulkanSupported())
        {
            std::cerr << "GLFW: Vulkan not supported!\n";
            return;
        }

        m_extensions = glfwGetRequiredInstanceExtensions(&m_extensions_count);
        m_windowHandle = windowHandle;
    }

    void GlfwVulkanRenderingBackend::SetupGraphicsAPI()
    {
        // Setup Vulkan
        GraphicsAPI::Vulkan::SetupVulkan(m_extensions, m_extensions_count);
        // Create Window Surface
        GraphicsAPI::Vulkan::AddWindowHandle(m_windowHandle);
    }

    void GlfwVulkanRenderingBackend::SetupWindow(int width, int height)
    {
        GraphicsAPI::Vulkan::SetupVulkanWindow(width, height);
    }

    bool GlfwVulkanRenderingBackend::NeedToResizeWindow()
    {
        return GraphicsAPI::Vulkan::NeedSwapChainRebuild();
    }

    void GlfwVulkanRenderingBackend::ResizeWindow(int width, int height)
    {
        GraphicsAPI::Vulkan::ResizeVulkanWindow(width, height);
        GraphicsAPI::Vulkan::SetSwapChainRebuildStatus(false);
    }

    void GlfwVulkanRenderingBackend::ConfigureImGui()
    {
        GraphicsAPI::Vulkan::ConfigureRendererBackend(m_windowHandle);
    }

    void GlfwVulkanRenderingBackend::StartImGuiFrame(const std::function<void()> &applicationMenubarCallback, const std::function<void()> &applicationUIRenderingCallback)
    {
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    void GlfwVulkanRenderingBackend::UploadFonts()
    {
        GraphicsAPI::Vulkan::UploadFonts();
    }

    void GlfwVulkanRenderingBackend::FrameRender(ImDrawData *draw_data)
    {
        GraphicsAPI::Vulkan::FrameRender(draw_data);
    }

    void GlfwVulkanRenderingBackend::FramePresent()
    {
        GraphicsAPI::Vulkan::FramePresent();
    }

    GLFWwindow *GlfwVulkanRenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

    void GlfwVulkanRenderingBackend::Shutdown()
    {
        GraphicsAPI::Vulkan::GraphicsDeviceWaitIdle();
        GraphicsAPI::Vulkan::FreeGraphicsResources();
        ImGui_ImplVulkan_Shutdown();
    }

    void GlfwVulkanRenderingBackend::Cleanup()
    {
        GraphicsAPI::Vulkan::CleanupVulkanWindow();
        GraphicsAPI::Vulkan::CleanupVulkan();
    }

    void GlfwVulkanRenderingBackend::SetClearColor(ImVec4 color)
    {
        GraphicsAPI::Vulkan::SetClearColor(color);
    }

}