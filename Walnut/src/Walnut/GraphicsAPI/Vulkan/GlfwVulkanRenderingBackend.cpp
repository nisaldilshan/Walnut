#include "VulkanRenderingBackend.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanGraphics.h"
#include <imgui_impl_glfw.h>
#include <iostream>

#include "VulkanImageRenderPipeline.h"

namespace Walnut
{
    VkRenderPass g_renderpass = VK_NULL_HANDLE;
    void CreateRenderPassForImagePipeline()
    {
        VkAttachmentDescription colorAtt{};
        colorAtt.format = VK_FORMAT_R8G8B8A8_UNORM;
        colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAtt.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkAttachmentReference colorAttRef{};
        colorAttRef.attachment = 0;
        colorAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAtt{};
        depthAtt.flags = 0;
        depthAtt.format = VK_FORMAT_D32_SFLOAT;
        depthAtt.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAtt.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttRef{};
        depthAttRef.attachment = 1;
        depthAttRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDesc{};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &colorAttRef;
        subpassDesc.pDepthStencilAttachment = &depthAttRef;

        VkSubpassDependency subpassDep{};
        subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDep.dstSubpass = 0;
        subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDep.srcAccessMask = 0;
        subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency depthDep{};
        depthDep.srcSubpass = VK_SUBPASS_EXTERNAL;
        depthDep.dstSubpass = 0;
        depthDep.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDep.srcAccessMask = 0;
        depthDep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency dependencies[] = {subpassDep, depthDep};
        VkAttachmentDescription attachments[] = {colorAtt, depthAtt};

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = attachments;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDesc;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        if (vkCreateRenderPass(GraphicsAPI::Vulkan::GetDevice(), &renderPassInfo, nullptr, &g_renderpass) != VK_SUCCESS)
        {
            std::cout << "error; could not create renderpass" << std::endl;
            assert(false);
        }
    }


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
        ImGui_ImplVulkan_SetMinImageCount(GraphicsAPI::Vulkan::GetMinImageCount());
        GraphicsAPI::Vulkan::ResizeVulkanWindow(width, height);
        GraphicsAPI::Vulkan::SetSwapChainRebuildStatus(false);
    }

    void VulkanRenderingBackend::ConfigureImGui()
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
    }

    void VulkanRenderingBackend::StartImGuiFrame()
    {
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    void VulkanRenderingBackend::FrameBegin()
    {
        GraphicsAPI::Vulkan::FrameBegin();
    }

    std::unique_ptr<GraphicsAPI::ImageRenderPipeline> g_imageRenderPipeline;
    void VulkanRenderingBackend::FrameRender(void* draw_data)
    {
        if (!g_renderpass) {
            CreateRenderPassForImagePipeline();
        }

        const auto& wd = GraphicsAPI::Vulkan::GetWindowData();

        if (!g_imageRenderPipeline)
        {
            // 1. Define your inline GLSL using raw string literals
            const std::string shadowVertexGLSL = R"(
                #version 450

                // Output to fragment shader
                layout(location = 0) out vec2 fragTexCoord;

                void main() {
                    // Generate UV coordinates: 
                    // Vertex 0: (0, 0)
                    // Vertex 1: (2, 0)
                    // Vertex 2: (0, 2)
                    fragTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
                    
                    // Map those UVs to Vulkan NDC positions:
                    // Vertex 0: (-1.0, -1.0)
                    // Vertex 1: ( 3.0, -1.0)
                    // Vertex 2: (-1.0,  3.0)
                    gl_Position = vec4(fragTexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
                }
            )";

            const std::string shadowFragmentGLSL = R"(
                #version 450

                // Input from vertex shader
                layout(location = 0) in vec2 fragTexCoord;

                // Matches your C++ layout: binding[0]
                layout(binding = 0) uniform sampler2D texSampler;

                // Output to the framebuffer
                layout(location = 0) out vec4 outColor;

                void main() {
                    outColor = texture(texSampler, fragTexCoord);
                }
            )";

            // 2. Compile GLSL to SPIR-V
            std::vector<uint32_t> vertSpirv = GraphicsAPI::compileGLSLToSPIRV_Vert(shadowVertexGLSL);
            std::vector<uint32_t> fragSpirv = GraphicsAPI::compileGLSLToSPIRV_Frag(shadowFragmentGLSL);

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

            VkDescriptorSetLayoutBinding binding[1] = {};
            binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding[0].descriptorCount = 1;
            binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkDescriptorSetLayoutCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.bindingCount = 1;
            info.pBindings = binding;
            VkDescriptorSetLayout descSetLayout;
            VkResult err = vkCreateDescriptorSetLayout(GraphicsAPI::Vulkan::GetDevice(), &info, GraphicsAPI::Vulkan::GetAllocator(), &descSetLayout);
            GraphicsAPI::Vulkan::check_vk_result(err);

            std::vector<VkDescriptorSetLayout> layouts{descSetLayout};
            GraphicsAPI::VertexInputLayout vertexInputLayout; // vertexInputLayout disabled                                       
            g_imageRenderPipeline = std::make_unique<GraphicsAPI::ImageRenderPipeline>(
                wd.RenderPass, layouts, vertexInputLayout, std::vector<VkPipelineShaderStageCreateInfo>{vertShaderStageInfo, fragShaderStageInfo});
        }

        const ImGui_ImplVulkanH_Frame* fd = &wd.Frames[wd.FrameIndex];

        // 1. Draw your image as the background
        vkCmdBindPipeline(fd->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_imageRenderPipeline->GetPipeline());

        if (!HasImageToRender()) {
            return;
        }

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
        vkCmdBindDescriptorSets(
            fd->CommandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            g_imageRenderPipeline->GetPipelineLayout(), 
            0, 1, reinterpret_cast<VkDescriptorSet*>(GetImageToRender()), 
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
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        g_imageRenderPipeline.reset();
    }

    void VulkanRenderingBackend::Cleanup()
    {
        GraphicsAPI::Vulkan::CleanupVulkanWindow();
        GraphicsAPI::Vulkan::CleanupVulkan();
    }

}