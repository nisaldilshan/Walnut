#include "VulkanRenderingBackend.h"

#include "VulkanGraphics.h"

#include "VulkanImageRenderPipeline.h"
#include <Walnut/Image.h>
#include "VulkanImage.h"

#include <imgui_impl_vulkan.h>

#include <iostream>
#include <cassert>

namespace Walnut
{ 
    VulkanRenderingBackend::VulkanRenderingBackend()
    {}

    VulkanRenderingBackend::~VulkanRenderingBackend()
    {}

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

    void VulkanRenderingBackend::FrameBegin()
    {
        GraphicsAPI::Vulkan::FrameBegin();
    }

    void VulkanRenderingBackend::FrameRender(std::unique_ptr<Image>& mainImage)
    {
        if (!mainImage) {
            assert(false);
            return;
        }

        assert(m_imageRenderPipeline != VK_NULL_HANDLE);
        vkCmdBindPipeline(GraphicsAPI::Vulkan::GetCurrentCommmandBuffer(), 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, m_imageRenderPipeline->GetPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(GraphicsAPI::Vulkan::GetWidth());
        viewport.height = static_cast<float>(GraphicsAPI::Vulkan::GetHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(GraphicsAPI::Vulkan::GetCurrentCommmandBuffer(), 0, 1, &viewport);

        const uint32_t w = GraphicsAPI::Vulkan::GetWidth();
        const uint32_t h = GraphicsAPI::Vulkan::GetHeight();
        VkRect2D scissor{{ 0, 0 }, { w, h }};
        vkCmdSetScissor(GraphicsAPI::Vulkan::GetCurrentCommmandBuffer(), 0, 1, &scissor);
        
        // Bind the descriptor set containing your VkImageView and a VkSampler
        auto descriptorSet = reinterpret_cast<VkDescriptorSet>(mainImage->GetHandle());
        vkCmdBindDescriptorSets(
            GraphicsAPI::Vulkan::GetCurrentCommmandBuffer(), 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            m_imageRenderPipeline->GetPipelineLayout(), 
            0, 1, &descriptorSet, 
            0, nullptr
        );

        // Draw the 3 vertices to trigger the vertex shader logic
        vkCmdDraw(GraphicsAPI::Vulkan::GetCurrentCommmandBuffer(), 3, 1, 0, 0);
    }

    void VulkanRenderingBackend::FrameRenderImGui(void *draw_data)
    {
        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData((ImDrawData*)draw_data, 
                                    GraphicsAPI::Vulkan::GetCurrentCommmandBuffer());
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

    void VulkanRenderingBackend::CreateMainImagePipeline(std::unique_ptr<Image>& mainImage)
    {
        auto& platformImage = mainImage->PlatformImageRef();
        std::vector<VkDescriptorSetLayout> layouts{platformImage->GetDescriptorSetLayout()};
        GraphicsAPI::VertexInputLayout vertexInputLayout; // vertexInputLayout disabled                                       
        m_imageRenderPipeline = std::make_unique<GraphicsAPI::VulkanImageRenderPipeline>(
            GraphicsAPI::Vulkan::GetRenderPass(), layouts, vertexInputLayout);
    }

    void VulkanRenderingBackend::DestroyMainImagePipeline()
    {
        m_imageRenderPipeline.reset();
    }

    void VulkanRenderingBackend::Shutdown()
    {
        GraphicsAPI::Vulkan::GraphicsDeviceWaitIdle();
        GraphicsAPI::Vulkan::FreeGraphicsResources();
    }

    void VulkanRenderingBackend::Cleanup()
    {
        GraphicsAPI::Vulkan::CleanupVulkanWindow();
        GraphicsAPI::Vulkan::CleanupVulkan();
    }

} // namespace Walnut