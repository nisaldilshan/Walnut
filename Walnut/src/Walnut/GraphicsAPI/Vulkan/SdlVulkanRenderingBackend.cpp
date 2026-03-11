#include "VulkanRenderingBackend.h"
#include <iostream>

#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <SDL3/SDL_vulkan.h>

#include "VulkanGraphics.h"

namespace Walnut
{
    VulkanRenderingBackend::VulkanRenderingBackend()
    {}

    VulkanRenderingBackend::~VulkanRenderingBackend()
    {}

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

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = GraphicsAPI::Vulkan::GetInstance();
        init_info.PhysicalDevice = GraphicsAPI::Vulkan::GetPhysicalDevice();
        init_info.Device = GraphicsAPI::Vulkan::GetDevice();
        init_info.QueueFamily = GraphicsAPI::Vulkan::GetQueueFamily();
        init_info.Queue = GraphicsAPI::Vulkan::GetQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = GraphicsAPI::Vulkan::GetDescriptorPool();
        init_info.MinImageCount = GraphicsAPI::Vulkan::GetMinImageCount();
        init_info.ImageCount = GraphicsAPI::Vulkan::GetWindowData().ImageCount;
        init_info.Allocator = GraphicsAPI::Vulkan::GetAllocator();
        init_info.CheckVkResultFn = [](VkResult err) {
            if (err == 0)
                return;
            fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
            if (err < 0)
                abort();
        };

        init_info.PipelineInfoMain.RenderPass = GraphicsAPI::Vulkan::GetWindowData().RenderPass;
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        ImGui_ImplVulkan_Init(&init_info);
        ImGui_ImplVulkan_SetMinImageCount(GraphicsAPI::Vulkan::GetMinImageCount());
    }

    void VulkanRenderingBackend::StartImGuiFrame()
    {
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
    }

    void VulkanRenderingBackend::FrameBegin()
    {
        GraphicsAPI::Vulkan::FrameBegin();
    }

    void VulkanRenderingBackend::FrameRender(void* draw_data)
    {
        const auto& wd = GraphicsAPI::Vulkan::GetWindowData();
        const ImGui_ImplVulkanH_Frame* fd = &wd.Frames[wd.FrameIndex];

        // 1. Draw your image as the background
        vkCmdBindPipeline(fd->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_FullscreenPipeline);
        
        // Bind the descriptor set containing your VkImageView and a VkSampler
        vkCmdBindDescriptorSets(
            fd->CommandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            m_PipelineLayout, 
            0, 1, &m_FinalImageDescriptorSet, 
            0, nullptr
        );

        // Draw the 3 vertices to trigger the vertex shader logic
        vkCmdDraw(fd->CommandBuffer, 3, 1, 0, 0);
    }

    void VulkanRenderingBackend::FrameRenderImGui(void *draw_data)
    {
        const auto& wd = GraphicsAPI::Vulkan::GetWindowData();
        
        const ImGui_ImplVulkanH_Frame* fd = &wd.Frames[wd.FrameIndex];

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData((ImDrawData*)draw_data, fd->CommandBuffer);
    }

    void VulkanRenderingBackend::FrameEnd()
    {
        GraphicsAPI::Vulkan::FrameEnd();
    }

    void VulkanRenderingBackend::FramePresent()
    {
        GraphicsAPI::Vulkan::FramePresent();
    }

    WalnutWindowHandleType *VulkanRenderingBackend::GetWindowHandle()
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