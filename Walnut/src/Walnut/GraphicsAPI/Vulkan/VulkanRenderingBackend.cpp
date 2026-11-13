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
        FrameRender(mainImage ? mainImage->GetHandle() : 0);
    }

    void VulkanRenderingBackend::FrameRender(uint64_t imageHandle)
    {
        if (imageHandle == 0)
            return;

        assert(m_imageRenderPipeline != nullptr);
        VkCommandBuffer commandBuffer = GraphicsAPI::Vulkan::GetCurrentCommmandBuffer();
        vkCmdBindPipeline(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, m_imageRenderPipeline->GetPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(GraphicsAPI::Vulkan::GetWidth());
        viewport.height = static_cast<float>(GraphicsAPI::Vulkan::GetHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        const uint32_t w = GraphicsAPI::Vulkan::GetWidth();
        const uint32_t h = GraphicsAPI::Vulkan::GetHeight();
        VkRect2D scissor{{ 0, 0 }, { w, h }};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        
        // Bind the descriptor set containing your VkImageView and a VkSampler
        auto descriptorSet = reinterpret_cast<VkDescriptorSet>(imageHandle);
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            m_imageRenderPipeline->GetPipelineLayout(), 
            0, 1, &descriptorSet, 
            0, nullptr
        );

        // Draw the 3 vertices to trigger the vertex shader logic
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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

    bool VulkanRenderingBackend::ReadFramePixels(std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height)
    {
        return GraphicsAPI::Vulkan::ReadSwapChainImage(pixels, width, height);
    }

    void VulkanRenderingBackend::FramePresent()
    {
        GraphicsAPI::Vulkan::FramePresent();
    }

    WalnutWindowHandleType *VulkanRenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

    void VulkanRenderingBackend::CreateMainImagePipeline()
    {
        VkDescriptorSetLayoutBinding binding[1] = {};
        binding[0].binding = 0;
        binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding[0].descriptorCount = 1;
        binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings = binding;

        VkResult err = vkCreateDescriptorSetLayout(
            GraphicsAPI::Vulkan::GetDevice(),
            &info,
            GraphicsAPI::Vulkan::GetAllocator(),
            &m_mainImageDescriptorSetLayout
        );
        GraphicsAPI::Vulkan::check_vk_result(err);

        std::vector<VkDescriptorSetLayout> layouts{m_mainImageDescriptorSetLayout};
        GraphicsAPI::VertexInputLayout vertexInputLayout; // vertexInputLayout disabled
        m_imageRenderPipeline = std::make_unique<GraphicsAPI::VulkanImageRenderPipeline>(
            GraphicsAPI::Vulkan::GetRenderPass(), layouts, vertexInputLayout);
    }

    void VulkanRenderingBackend::DestroyMainImagePipeline()
    {
        m_imageRenderPipeline.reset();
        if (m_mainImageDescriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(
                GraphicsAPI::Vulkan::GetDevice(),
                m_mainImageDescriptorSetLayout,
                GraphicsAPI::Vulkan::GetAllocator()
            );
            m_mainImageDescriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderingBackend::WaitIdle()
    {
        GraphicsAPI::Vulkan::GraphicsDeviceWaitIdle();
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
