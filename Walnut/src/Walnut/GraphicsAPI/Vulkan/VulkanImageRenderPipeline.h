#pragma once
#include "VulkanGraphics.h"
#include <string>

namespace GraphicsAPI
{

struct VertexInputLayout
{
    bool enabled = false;
    VkVertexInputBindingDescription m_vertexBindingDescs;
    std::vector<VkVertexInputAttributeDescription> m_vertexAttribDescs;
};

std::vector<uint32_t> compileGLSLToSPIRV_Vert(const std::string& source);
std::vector<uint32_t> compileGLSLToSPIRV_Frag(const std::string& source);
VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t>& spirv);

class ImageRenderPipeline
{

public:
    ImageRenderPipeline(VkRenderPass renderPass, 
                        std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                        const VertexInputLayout& vertexInputLayout, 
                        const std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos);
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

    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_Pipeline;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageInfos;
};


} // namespace GraphicsAPI