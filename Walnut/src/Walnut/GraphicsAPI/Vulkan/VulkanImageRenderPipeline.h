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

class ImageRenderPipeline
{

public:
    ImageRenderPipeline(VkRenderPass renderPass, 
                        std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                        const VertexInputLayout& vertexInputLayout);
    ~ImageRenderPipeline();

    ImageRenderPipeline(const ImageRenderPipeline&) = delete;
    ImageRenderPipeline& operator=(const ImageRenderPipeline&) = delete;
    ImageRenderPipeline(ImageRenderPipeline&&) = delete;
    ImageRenderPipeline& operator=(ImageRenderPipeline&&) = delete;

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