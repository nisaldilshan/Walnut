#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../../Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h"

namespace GraphicsAPI
{
    class WebGPURenderer2D
    {
    public:
        WebGPURenderer2D() = default;
        ~WebGPURenderer2D() = default;

        void CreateTextureToRenderInto(uint32_t width, uint32_t height);
        void CreateShaders(const char* shaderSource);
        void CreateStandaloneShader(const char *shaderSource, uint32_t vertexShaderCallCount);
        void CreatePipeline();
        void CreateVertexBuffer(const std::vector<float>& bufferData, wgpu::VertexBufferLayout bufferLayout);
        void CreateIndexBuffer(const std::vector<uint16_t> &bufferData);
        void SetBindGroupLayoutEntry(wgpu::BindGroupLayoutEntry bindGroupLayoutEntry);
        void CreateBindGroup();
        void CreateUniformBuffer(const std::vector<float>& bufferData);
        void Render();
        ImTextureID GetDescriptorSet();
    private:
        wgpu::ShaderModule m_shaderModule = nullptr;
        wgpu::RenderPipeline m_pipeline = nullptr;
        wgpu::TextureView m_nextTexture = nullptr;

        uint32_t m_vertexCount = 0;
        uint64_t m_vertexBufferSize = 0;
        wgpu::Buffer m_vertexBuffer = nullptr;
        wgpu::VertexBufferLayout m_vertexBufferLayout;

        uint32_t m_indexCount = 0;
        wgpu::Buffer m_indexBuffer = nullptr;

        wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
        wgpu::PipelineLayout m_pipelineLayout = nullptr;
        wgpu::Buffer m_uniformBuffer = nullptr;
        wgpu::BindGroup m_bindGroup = nullptr;

        uint32_t m_width, m_height;
    };
}