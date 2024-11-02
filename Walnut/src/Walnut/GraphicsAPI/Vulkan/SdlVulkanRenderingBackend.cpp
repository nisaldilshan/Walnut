#include "VulkanRenderingBackend.h"
#include <iostream>

#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL_vulkan.h>

#include "VulkanGraphics.h"

namespace Walnut
{

    void VulkanRenderingBackend::Init(WindowHandleType* windowHandle)
    {
        auto result = SDL_Vulkan_GetInstanceExtensions(windowHandle, &m_extensions_count, nullptr);
        if (result != SDL_TRUE)
        {
            std::cerr << "SDL: Failed to get Vulkan instance extensions count!\n";
            return;
        }
        m_extensions = new const char*[m_extensions_count];
        result = SDL_Vulkan_GetInstanceExtensions(windowHandle, &m_extensions_count, m_extensions);
        if (result != SDL_TRUE)
        {
            std::cerr << "SDL: Failed to get Vulkan instance extensions!\n";
            return;
        }
        m_windowHandle = windowHandle;

        // Setup Vulkan
        GraphicsAPI::Vulkan::SetupVulkan(m_extensions, m_extensions_count);
        delete [] m_extensions;
        // Create Window Surface
        result = SDL_Vulkan_CreateSurface(windowHandle, 
                                            GraphicsAPI::Vulkan::GetInstance(), 
                                            GraphicsAPI::Vulkan::GetSurface());
        if (result != SDL_TRUE)
        {
            std::cerr << "SDL: Failed to create Vulkan surface!\n";
            return;
        }
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
        ImGui_ImplSDL2_InitForVulkan(m_windowHandle);
        GraphicsAPI::Vulkan::ConfigureRendererBackend();
    }

    void VulkanRenderingBackend::StartImGuiFrame()
    {
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
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

    WindowHandleType* VulkanRenderingBackend::GetWindowHandle()
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