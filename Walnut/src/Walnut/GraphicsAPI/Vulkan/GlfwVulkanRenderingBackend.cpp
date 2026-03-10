#include "VulkanRenderingBackend.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanGraphics.h"
#include <imgui_impl_glfw.h>

#include "VulkanImageRenderPipeline.h"
#include <Walnut/Image.h>
#include "VulkanImage.h"

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

    void VulkanRenderingBackend::CreateImGuiPipeline()
    {
        ImGui_ImplGlfw_InitForVulkan(m_windowHandle, true);
        
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

    std::unique_ptr<GraphicsAPI::ImageRenderPipeline> g_imageRenderPipeline;
    void VulkanRenderingBackend::CreateMainImagePipeline(std::unique_ptr<Image> &mainImage)
    {
        if (!g_imageRenderPipeline)
        {
            std::vector<uint32_t> vertSpirv = GraphicsAPI::compileGLSLToSPIRV_Vert();
            std::vector<uint32_t> fragSpirv = GraphicsAPI::compileGLSLToSPIRV_Frag();

            // 3. Create VkShaderModules (Assuming you have access to your 'VkDevice device')
            VkShaderModule vertShaderModule = GraphicsAPI::createShaderModule(GraphicsAPI::Vulkan::GetDevice(), vertSpirv);
            VkShaderModule fragShaderModule = GraphicsAPI::createShaderModule(GraphicsAPI::Vulkan::GetDevice(), fragSpirv);

            // 4. Create the Shader Stage Create Infos
            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            auto& platformImage = mainImage->PlatformImageRef();
            std::vector<VkDescriptorSetLayout> layouts{platformImage->GetDescriptorSetLayout()};
            GraphicsAPI::VertexInputLayout vertexInputLayout; // vertexInputLayout disabled                                       
            g_imageRenderPipeline = std::make_unique<GraphicsAPI::ImageRenderPipeline>(
                GraphicsAPI::Vulkan::GetWindowData().RenderPass, layouts, vertexInputLayout, 
                std::vector<VkPipelineShaderStageCreateInfo>{vertShaderStageInfo, fragShaderStageInfo});
        }
    }

    void VulkanRenderingBackend::StartImGuiFrame()
    {
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
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

        const auto& wd = GraphicsAPI::Vulkan::GetWindowData();
        const ImGui_ImplVulkanH_Frame* fd = &wd.Frames[wd.FrameIndex];

        assert(g_imageRenderPipeline != VK_NULL_HANDLE);
        vkCmdBindPipeline(fd->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_imageRenderPipeline->GetPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(wd.Width);
        viewport.height = static_cast<float>(wd.Height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(fd->CommandBuffer, 0, 1, &viewport);

        const uint32_t w = wd.Width;
        const uint32_t h = wd.Height;
        VkRect2D scissor{{ 0, 0 }, { w, h }};
        vkCmdSetScissor(fd->CommandBuffer, 0, 1, &scissor);
        
        // Bind the descriptor set containing your VkImageView and a VkSampler
        auto descriptorSet = reinterpret_cast<VkDescriptorSet>(mainImage->GetDescriptorSet());
        vkCmdBindDescriptorSets(
            fd->CommandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            g_imageRenderPipeline->GetPipelineLayout(), 
            0, 1, &descriptorSet, 
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

    GLFWwindow *VulkanRenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

    void VulkanRenderingBackend::Shutdown()
    {
        GraphicsAPI::Vulkan::GraphicsDeviceWaitIdle();
        GraphicsAPI::Vulkan::FreeGraphicsResources();
        g_imageRenderPipeline.reset();
    }

    void VulkanRenderingBackend::DestroyImGuiPipeline()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void VulkanRenderingBackend::Cleanup()
    {
        GraphicsAPI::Vulkan::CleanupVulkanWindow();
        GraphicsAPI::Vulkan::CleanupVulkan();
    }

}