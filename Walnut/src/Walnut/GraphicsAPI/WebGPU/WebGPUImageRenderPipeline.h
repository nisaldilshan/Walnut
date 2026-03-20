#pragma once
#include "WebGPUGraphics.h"

namespace GraphicsAPI
{

class WebGPUImageRenderPipeline
{

public:
    WebGPUImageRenderPipeline(std::vector<wgpu::BindGroupLayout>& descriptorSetLayouts);
    ~WebGPUImageRenderPipeline();

    WebGPUImageRenderPipeline(const WebGPUImageRenderPipeline&) = delete;
    WebGPUImageRenderPipeline& operator=(const WebGPUImageRenderPipeline&) = delete;
    WebGPUImageRenderPipeline(WebGPUImageRenderPipeline&&) = delete;
    WebGPUImageRenderPipeline& operator=(WebGPUImageRenderPipeline&&) = delete;

    wgpu::RenderPipeline GetPipeline() const { return m_pipeline; }
    wgpu::PipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
    wgpu::BindGroup GetBindGroup() const { return m_bindGroup; }

private:
    void CreatePipelineLayout(const std::vector<wgpu::BindGroupLayout>& descriptorSetLayouts);
    void CreatePipeline();
    void PrepareShaders();

    wgpu::PipelineLayout m_pipelineLayout = nullptr;
    wgpu::RenderPipeline m_pipeline = nullptr;
    wgpu::ShaderModule m_shaderModule = nullptr;
    wgpu::BindGroup m_bindGroup = nullptr;
};


} // namespace GraphicsAPI