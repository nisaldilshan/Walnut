#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../../Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h"

namespace GraphicsAPI
{
    class WebGPURenderer3D
    {
    public:
        WebGPURenderer3D() = default;
        ~WebGPURenderer3D() = default;

        void CreateTextureToRenderInto(uint32_t width, uint32_t height);
        void CreateShaders(const char* shaderSource);
        void CreateStandaloneShader(const char *shaderSource, uint32_t vertexShaderCallCount);
        void CreatePipeline();
        void CreateVertexBuffer(const void* bufferData, uint32_t bufferLength, wgpu::VertexBufferLayout bufferLayout);
        void CreateIndexBuffer(const std::vector<uint16_t> &bufferData);
        void SetBindGroupLayoutEntry(wgpu::BindGroupLayoutEntry bindGroupLayoutEntry);
        void SetSizeOfUniform(uint32_t sizeOfUniform);
        void CreateBindGroup();
        void CreateUniformBuffer(size_t dynamicOffsetCount);
        void CreateDepthTexture();
        void SetUniformData(const void* bufferData, uint32_t uniformIndex);
        void SimpleRender();
        void Render(uint32_t uniformIndex);
        void RenderIndexed(uint32_t uniformIndex);
        ImTextureID GetDescriptorSet();
        void BeginRenderPass();
        void EndRenderPass();
        void Reset();
        
    private:
        void SubmitCommandBuffer();
        uint32_t GetOffset(uint32_t uniformIndex);

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
        size_t m_dynamicOffsetCount = 0;
        uint32_t m_sizeOfUniform = 0;

        wgpu::CommandEncoder m_currentCommandEncoder = nullptr;
        wgpu::RenderPassEncoder m_renderPass = nullptr;

        wgpu::TextureFormat m_depthTextureFormat =  wgpu::TextureFormat::Undefined;
        wgpu::Texture m_depthTexture = nullptr;
        wgpu::TextureView m_depthTextureView = nullptr;

        wgpu::Limits m_deviceLimits;

        uint32_t m_width, m_height;
    };
}