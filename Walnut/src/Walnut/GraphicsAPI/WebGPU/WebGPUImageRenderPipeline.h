#pragma once
#include "WebGPUGraphics.h"

namespace GraphicsAPI
{

class WebGPUImageRenderPipeline
{

public:
    WebGPUImageRenderPipeline(wgpu::RenderPassEncoder renderPass, 
                        std::vector<wgpu::BindGroupLayout>& descriptorSetLayouts);
    ~WebGPUImageRenderPipeline();

    WebGPUImageRenderPipeline(const WebGPUImageRenderPipeline&) = delete;
    WebGPUImageRenderPipeline& operator=(const WebGPUImageRenderPipeline&) = delete;
    WebGPUImageRenderPipeline(WebGPUImageRenderPipeline&&) = delete;
    WebGPUImageRenderPipeline& operator=(WebGPUImageRenderPipeline&&) = delete;

    wgpu::RenderPipeline GetPipeline() const { return m_Pipeline; }
    wgpu::PipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

private:
    void CreatePipelineLayout(const std::vector<wgpu::BindGroupLayout>& descriptorSetLayouts);
    void CreatePipeline(wgpu::RenderPassEncoder renderPass);
    void PrepareShaders();

    wgpu::PipelineLayout m_PipelineLayout;
    wgpu::RenderPipeline m_Pipeline;
    wgpu::ShaderModule m_vertexShader;
    wgpu::ShaderModule m_fragmentShader;
};


} // namespace GraphicsAPI