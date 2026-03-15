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

private:
    void CreatePipelineLayout(const std::vector<wgpu::BindGroupLayout>& descriptorSetLayouts);
    void CreatePipeline();
    void PrepareShaders();

    wgpu::PipelineLayout m_pipelineLayout;
    wgpu::RenderPipeline m_pipeline;
    wgpu::ShaderModule m_vertexShader;
    wgpu::ShaderModule m_fragmentShader;
};


} // namespace GraphicsAPI