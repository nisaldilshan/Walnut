#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../../Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h"

namespace GraphicsAPI
{
    class WebGPURenderer2D
    {
        typedef int VkBuffer;
    public:
        WebGPURenderer2D() = default;
        ~WebGPURenderer2D() = default;

        size_t CreateUploadBuffer(size_t upload_size);
        void CreateSwapChain(uint32_t width, uint32_t height);
        void CreateShaders();
        void CreatePipeline();
        void Render();
        void CreateDescriptorSet();
        ImTextureID GetDescriptorSet();
        bool ImageAvailable();
        void ResourceFree();
        VkBuffer GetStagingBuffer();
    private:
        wgpu::SwapChain m_rendererSwapChain = nullptr;
        wgpu::ShaderModule m_shaderModule = nullptr;
        wgpu::RenderPipeline m_pipeline = nullptr;
        uint32_t m_width, m_height;
        uint32_t m_DataFormat;
        void* m_imageBuffer;
    };
}