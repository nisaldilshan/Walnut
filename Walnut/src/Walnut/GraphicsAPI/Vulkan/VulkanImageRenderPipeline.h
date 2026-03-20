#pragma once
#include "VulkanGraphics.h"

namespace GraphicsAPI
{

struct VertexInputLayout
{
    bool enabled = false;
    VkVertexInputBindingDescription m_vertexBindingDescs;
    std::vector<VkVertexInputAttributeDescription> m_vertexAttribDescs;
};

class VulkanImageRenderPipeline
{

public:
    VulkanImageRenderPipeline(VkRenderPass renderPass, 
                        std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                        const VertexInputLayout& vertexInputLayout);
    ~VulkanImageRenderPipeline();

    VulkanImageRenderPipeline(const VulkanImageRenderPipeline&) = delete;
    VulkanImageRenderPipeline& operator=(const VulkanImageRenderPipeline&) = delete;
    VulkanImageRenderPipeline(VulkanImageRenderPipeline&&) = delete;
    VulkanImageRenderPipeline& operator=(VulkanImageRenderPipeline&&) = delete;

    VkPipeline GetPipeline() const { return m_Pipeline; }
    VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

private:
    void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
    void CreatePipeline(VkRenderPass renderPass, const VertexInputLayout &vertexInputLayout);
    void PrepareShaders();

    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_Pipeline;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageInfos;
};


} // namespace GraphicsAPI