#include "VulkanImageRenderPipeline.h"
#include <shaderc/shaderc.hpp>

namespace GraphicsAPI
{

// Helper 1: Compiles inline GLSL string to SPIR-V bytecode
std::vector<uint32_t> compileGLSLToSPIRV_Vert(const std::string& source) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    
    // Optimize for performance (optional)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, shaderc_glsl_vertex_shader, "Image-Vertex", options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        //std::cerr << "Shader Compilation Error in " << name << ": " << module.GetErrorMessage() << std::endl;
        assert(false);
        return {};
    }

    return {module.cbegin(), module.cend()};
}

std::vector<uint32_t> compileGLSLToSPIRV_Frag(const std::string& source) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    
    // Optimize for performance (optional)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, shaderc_glsl_fragment_shader, "Image-Fragment", options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        //std::cerr << "Shader Compilation Error in " << name << ": " << module.GetErrorMessage() << std::endl;
        assert(false);
        return {};
    }

    return {module.cbegin(), module.cend()};
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
                                    const std::vector<VkPipelineShaderStageCreateInfo> &shaderStageInfos)
    : m_PipelineLayout(VK_NULL_HANDLE), 
      m_Pipeline(VK_NULL_HANDLE)
{
    CreatePipelineLayout(descriptorSetLayouts);
    CreatePipeline(renderPass, vertexInputLayout, shaderStageInfos);
}

ImageRenderPipeline::~ImageRenderPipeline()
{
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

void ImageRenderPipeline::CreatePipeline(VkRenderPass renderPass, const VertexInputLayout &vertexInputLayout, 
                                    const std::vector<VkPipelineShaderStageCreateInfo> &shaderStageInfos)
{
    assert(m_PipelineLayout != VK_NULL_HANDLE);

    //std::cout << "Creating render pipeline..." << std::endl;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::vector<VkVertexInputBindingDescription> vertexBindingDescs;
    std::vector<VkVertexInputAttributeDescription> vertexAttribDescs;
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

    assert(shaderStageInfos.size() > 0);
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = shaderStageInfos.size();
    pipelineCreateInfo.pStages = shaderStageInfos.data();
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