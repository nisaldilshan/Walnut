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
        void CreatePipeline();
        void Render();
        ImTextureID GetDescriptorSet();
    private:
        wgpu::ShaderModule m_shaderModule = nullptr;
        wgpu::RenderPipeline m_pipeline = nullptr;
        wgpu::TextureView m_nextTexture = nullptr;
        uint32_t m_width, m_height;
    };
}