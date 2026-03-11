#include "VulkanImageRenderPipeline.h"
#include <shaderc/shaderc.hpp>

namespace GraphicsAPI
{

// Helper 1: Compiles inline GLSL string to SPIR-V bytecode
std::vector<uint32_t> compileGLSLToSPIRV_Vert() {
    // 1. Define your inline GLSL using raw string literals
    const std::string vertexSource = R"(
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


    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    
    // Optimize for performance (optional)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(vertexSource, shaderc_glsl_vertex_shader, "Image-Vertex", options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        //std::cerr << "Shader Compilation Error in " << name << ": " << module.GetErrorMessage() << std::endl;
        assert(false);
        return {};
    }

    return {module.cbegin(), module.cend()};
}

std::vector<uint32_t> getSPIRV_Vert()
{
    // const std::string vertexSource = R"(
    //     #version 450

    //     // Output to fragment shader
    //     layout(location = 0) out vec2 fragTexCoord;

    //     void main() {
    //         // Generate UV coordinates: 
    //         // Vertex 0: (0, 0)
    //         // Vertex 1: (2, 0)
    //         // Vertex 2: (0, 2)
    //         fragTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
            
    //         // Map those UVs to Vulkan NDC positions:
    //         // Vertex 0: (-1.0, -1.0)
    //         // Vertex 1: ( 3.0, -1.0)
    //         // Vertex 2: (-1.0,  3.0)
    //         gl_Position = vec4(fragTexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
    //     }
    // )";

    // how to get this SPIR-V binary blob:
    // glslc shader.vert -O -mfmt=c -o vert.h
    return {0x07230203, 0x00010000, 0x000d000b, 0x0000002c,
            0x00000000, 0x00020011, 0x00000001, 0x0006000b,
            0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
            0x00000000, 0x0003000e, 0x00000000, 0x00000001,
            0x0008000f, 0x00000000, 0x00000004, 0x6e69616d,
            0x00000000, 0x00000009, 0x0000000c, 0x0000001d,
            0x00040047, 0x00000009, 0x0000001e, 0x00000000,
            0x00040047, 0x0000000c, 0x0000000b, 0x0000002a,
            0x00050048, 0x0000001b, 0x00000000, 0x0000000b,
            0x00000000, 0x00050048, 0x0000001b, 0x00000001,
            0x0000000b, 0x00000001, 0x00050048, 0x0000001b,
            0x00000002, 0x0000000b, 0x00000003, 0x00050048,
            0x0000001b, 0x00000003, 0x0000000b, 0x00000004,
            0x00030047, 0x0000001b, 0x00000002, 0x00020013,
            0x00000002, 0x00030021, 0x00000003, 0x00000002,
            0x00030016, 0x00000006, 0x00000020, 0x00040017,
            0x00000007, 0x00000006, 0x00000002, 0x00040020,
            0x00000008, 0x00000003, 0x00000007, 0x0004003b,
            0x00000008, 0x00000009, 0x00000003, 0x00040015,
            0x0000000a, 0x00000020, 0x00000001, 0x00040020,
            0x0000000b, 0x00000001, 0x0000000a, 0x0004003b,
            0x0000000b, 0x0000000c, 0x00000001, 0x0004002b,
            0x0000000a, 0x0000000e, 0x00000001, 0x0004002b,
            0x0000000a, 0x00000010, 0x00000002, 0x00040017,
            0x00000017, 0x00000006, 0x00000004, 0x00040015,
            0x00000018, 0x00000020, 0x00000000, 0x0004002b,
            0x00000018, 0x00000019, 0x00000001, 0x0004001c,
            0x0000001a, 0x00000006, 0x00000019, 0x0006001e,
            0x0000001b, 0x00000017, 0x00000006, 0x0000001a,
            0x0000001a, 0x00040020, 0x0000001c, 0x00000003,
            0x0000001b, 0x0004003b, 0x0000001c, 0x0000001d,
            0x00000003, 0x0004002b, 0x0000000a, 0x0000001e,
            0x00000000, 0x0004002b, 0x00000006, 0x00000020,
            0x40000000, 0x0004002b, 0x00000006, 0x00000022,
            0x3f800000, 0x0004002b, 0x00000006, 0x00000025,
            0x00000000, 0x00040020, 0x00000029, 0x00000003,
            0x00000017, 0x0005002c, 0x00000007, 0x0000002b,
            0x00000022, 0x00000022, 0x00050036, 0x00000002,
            0x00000004, 0x00000000, 0x00000003, 0x000200f8,
            0x00000005, 0x0004003d, 0x0000000a, 0x0000000d,
            0x0000000c, 0x000500c4, 0x0000000a, 0x0000000f,
            0x0000000d, 0x0000000e, 0x000500c7, 0x0000000a,
            0x00000011, 0x0000000f, 0x00000010, 0x0004006f,
            0x00000006, 0x00000012, 0x00000011, 0x000500c7,
            0x0000000a, 0x00000014, 0x0000000d, 0x00000010,
            0x0004006f, 0x00000006, 0x00000015, 0x00000014,
            0x00050050, 0x00000007, 0x00000016, 0x00000012,
            0x00000015, 0x0003003e, 0x00000009, 0x00000016,
            0x0004003d, 0x00000007, 0x0000001f, 0x00000009,
            0x0005008e, 0x00000007, 0x00000021, 0x0000001f,
            0x00000020, 0x00050083, 0x00000007, 0x00000024,
            0x00000021, 0x0000002b, 0x00050051, 0x00000006,
            0x00000026, 0x00000024, 0x00000000, 0x00050051,
            0x00000006, 0x00000027, 0x00000024, 0x00000001,
            0x00070050, 0x00000017, 0x00000028, 0x00000026,
            0x00000027, 0x00000025, 0x00000022, 0x00050041,
            0x00000029, 0x0000002a, 0x0000001d, 0x0000001e,
            0x0003003e, 0x0000002a, 0x00000028, 0x000100fd,
            0x00010038};
}

std::vector<uint32_t> compileGLSLToSPIRV_Frag() {
    const std::string fragmentSource = R"(
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

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    
    // Optimize for performance (optional)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(fragmentSource, shaderc_glsl_fragment_shader, "Image-Fragment", options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        //std::cerr << "Shader Compilation Error in " << name << ": " << module.GetErrorMessage() << std::endl;
        assert(false);
        return {};
    }

    return {module.cbegin(), module.cend()};
}

std::vector<uint32_t> getSPIRV_Frag()
{
    return {0x07230203, 0x00010000, 0x000d000b, 0x00000014,
            0x00000000, 0x00020011, 0x00000001, 0x0006000b,
            0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
            0x00000000, 0x0003000e, 0x00000000, 0x00000001,
            0x0007000f, 0x00000004, 0x00000004, 0x6e69616d,
            0x00000000, 0x00000009, 0x00000011, 0x00030010,
            0x00000004, 0x00000007, 0x00040047, 0x00000009,
            0x0000001e, 0x00000000, 0x00040047, 0x0000000d,
            0x00000022, 0x00000000, 0x00040047, 0x0000000d,
            0x00000021, 0x00000000, 0x00040047, 0x00000011,
            0x0000001e, 0x00000000, 0x00020013, 0x00000002,
            0x00030021, 0x00000003, 0x00000002, 0x00030016,
            0x00000006, 0x00000020, 0x00040017, 0x00000007,
            0x00000006, 0x00000004, 0x00040020, 0x00000008,
            0x00000003, 0x00000007, 0x0004003b, 0x00000008,
            0x00000009, 0x00000003, 0x00090019, 0x0000000a,
            0x00000006, 0x00000001, 0x00000000, 0x00000000,
            0x00000000, 0x00000001, 0x00000000, 0x0003001b,
            0x0000000b, 0x0000000a, 0x00040020, 0x0000000c,
            0x00000000, 0x0000000b, 0x0004003b, 0x0000000c,
            0x0000000d, 0x00000000, 0x00040017, 0x0000000f,
            0x00000006, 0x00000002, 0x00040020, 0x00000010,
            0x00000001, 0x0000000f, 0x0004003b, 0x00000010,
            0x00000011, 0x00000001, 0x00050036, 0x00000002,
            0x00000004, 0x00000000, 0x00000003, 0x000200f8,
            0x00000005, 0x0004003d, 0x0000000b, 0x0000000e,
            0x0000000d, 0x0004003d, 0x0000000f, 0x00000012,
            0x00000011, 0x00050057, 0x00000007, 0x00000013,
            0x0000000e, 0x00000012, 0x0003003e, 0x00000009,
            0x00000013, 0x000100fd, 0x00010038};
}

// Helper 2: Creates a Vulkan Shader Module from SPIR-V
VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t>& spirv) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t); // Size must be in bytes
    createInfo.pCode = spirv.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        //std::cerr << "error: failed to create shader module!" << std::endl;
        assert(false);
    }
    return shaderModule;
}

VkPipelineColorBlendStateCreateInfo CreateColorBlendState(VkPipelineColorBlendAttachmentState &colorBlendAttachment)
{
    VkPipelineColorBlendStateCreateInfo colorBlendingInfo;
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingInfo.pNext = nullptr;
    colorBlendingInfo.flags = 0;
    colorBlendingInfo.logicOpEnable = VK_FALSE;
    colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &colorBlendAttachment;
    colorBlendingInfo.blendConstants[0] = 0.0f;
    colorBlendingInfo.blendConstants[1] = 0.0f;
    colorBlendingInfo.blendConstants[2] = 0.0f;
    colorBlendingInfo.blendConstants[3] = 0.0f;
    return colorBlendingInfo;
}

VkPipelineRasterizationStateCreateInfo getRasterizerInfo()
{
    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    // because we use negative viewports in vulkan main pipeline (not in shadow)
    rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; 
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    return rasterizerInfo;
}

ImageRenderPipeline::ImageRenderPipeline(VkRenderPass renderPass, 
                                    std::vector<VkDescriptorSetLayout> &descriptorSetLayouts, 
                                    const VertexInputLayout &vertexInputLayout, 
                                    const std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos)
    : m_PipelineLayout(VK_NULL_HANDLE), 
      m_Pipeline(VK_NULL_HANDLE),
      m_shaderStageInfos(shaderStageInfos)
{
    CreatePipelineLayout(descriptorSetLayouts);
    CreatePipeline(renderPass, vertexInputLayout);
}

ImageRenderPipeline::~ImageRenderPipeline()
{
    for (const auto &shaderStageInfo : m_shaderStageInfos)
    {
        if (shaderStageInfo.module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(Vulkan::GetDevice(), shaderStageInfo.module, nullptr);
        }
    }
    m_shaderStageInfos.clear();

    if (m_Pipeline)
    {
        vkDestroyPipeline(Vulkan::GetDevice(), m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
    }

    if (m_PipelineLayout)
    {
        vkDestroyPipelineLayout(Vulkan::GetDevice(), m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }
}

void ImageRenderPipeline::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    auto result = vkCreatePipelineLayout(Vulkan::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
    if (result != VK_SUCCESS)
    {
        Vulkan::check_vk_result(result);
    }
}

void ImageRenderPipeline::CreatePipeline(VkRenderPass renderPass, const VertexInputLayout &vertexInputLayout)
{
    assert(m_PipelineLayout != VK_NULL_HANDLE);

    //std::cout << "Creating render pipeline..." << std::endl;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::vector<VkVertexInputBindingDescription> vertexBindingDescs;
    std::vector<VkVertexInputAttributeDescription> vertexAttribDescs;
    if (vertexInputLayout.enabled) {
        
        assert(vertexInputLayout.m_vertexAttribDescs.size() > 0);
        vertexBindingDescs.push_back(vertexInputLayout.m_vertexBindingDescs);
        for (const auto &vertextAttribDesc : vertexInputLayout.m_vertexAttribDescs)
        {
            vertexAttribDescs.push_back(vertextAttribDesc);
        }
        vertexInputInfo.vertexBindingDescriptionCount = vertexBindingDescs.size();
        vertexInputInfo.pVertexBindingDescriptions = vertexBindingDescs.data();
        vertexInputInfo.vertexAttributeDescriptionCount = vertexAttribDescs.size();
        vertexInputInfo.pVertexAttributeDescriptions = vertexAttribDescs.data();
    } else {
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    }
    
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = nullptr;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = nullptr;

    VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Use the new alpha directly
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Discard the old alpha
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendingInfo = CreateColorBlendState(colorBlendAttachment);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.minDepthBounds = 0.0f;
    depthStencilInfo.maxDepthBounds = 1.0f;
    depthStencilInfo.stencilTestEnable = VK_FALSE;

    std::vector<VkDynamicState> dynStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynStatesInfo{};
    dynStatesInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynStatesInfo.dynamicStateCount = static_cast<uint32_t>(dynStates.size());
    dynStatesInfo.pDynamicStates = dynStates.data();

    assert(m_shaderStageInfos.size() > 0);
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = m_shaderStageInfos.size();
    pipelineCreateInfo.pStages = m_shaderStageInfos.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pViewportState = &viewportStateInfo;
    VkPipelineRasterizationStateCreateInfo rasterizerInfo = getRasterizerInfo();
    pipelineCreateInfo.pRasterizationState = &rasterizerInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendingInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineCreateInfo.pDynamicState = &dynStatesInfo;
    pipelineCreateInfo.layout = m_PipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(GraphicsAPI::Vulkan::GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
        //std::cout << "error: could not create rendering pipeline" << std::endl;
    }

    // can save memory by calling DestroyShaders() after pipeline have been created
    // currently not possible, as pipeline get recreated every window get resized
    assert(m_Pipeline != VK_NULL_HANDLE);
    //std::cout << "Render pipeline: " << m_Pipeline << std::endl;
}

}