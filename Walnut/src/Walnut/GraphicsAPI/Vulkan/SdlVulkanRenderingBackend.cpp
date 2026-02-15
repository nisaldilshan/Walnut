#include "VulkanRenderingBackend.h"
#include <iostream>

#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <SDL3/SDL_vulkan.h>

#include "VulkanGraphics.h"

namespace Walnut
{

    void VulkanRenderingBackend::Init(WalnutWindowHandleType* windowHandle)
    {
        uint32_t sdl_extensions_count = 0;
        const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
        if (*sdl_extensions == nullptr)
        {
            std::cerr << "SDL: Failed to get Vulkan instance extensions count!\n";
            assert(false);
            return;
        }
        
        ImVector<const char*> extensions;
        for (uint32_t n = 0; n < sdl_extensions_count; n++) {
            // if (std::string(sdl_extensions[n]) == "VK_KHR_portability_enumeration") { 
            //     // TODO: somehow vkCreateInstance function fails when this extension is present
            //     std::cout << "   SDL: Extension - " << sdl_extensions[n] << " skipping" << std::endl;
            //     continue;
            // }
            // else 
            {
                std::cout << "   SDL: Extension - " << sdl_extensions[n] << std::endl;
            }
            extensions.push_back(sdl_extensions[n]);
        }
        
        
        m_windowHandle = windowHandle;

        // Setup Vulkan
        GraphicsAPI::Vulkan::SetupVulkan(extensions);
        // Create Window Surface
        const bool result = SDL_Vulkan_CreateSurface(windowHandle, 
                                            GraphicsAPI::Vulkan::GetInstance(), 
                                            NULL,
                                            GraphicsAPI::Vulkan::GetSurface());
        if (result != true)
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
        ImGui_ImplSDL3_InitForVulkan(m_windowHandle);
        GraphicsAPI::Vulkan::ConfigureRendererBackend();
    }

    void VulkanRenderingBackend::StartImGuiFrame()
    {
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
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

    WalnutWindowHandleType* VulkanRenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

    void VulkanRenderingBackend::Shutdown()
    {
        GraphicsAPI::Vulkan::GraphicsDeviceWaitIdle();
        GraphicsAPI::Vulkan::FreeGraphicsResources();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
    }

    void VulkanRenderingBackend::Cleanup()
    {
        GraphicsAPI::Vulkan::CleanupVulkanWindow();
        GraphicsAPI::Vulkan::CleanupVulkan();
    }

}