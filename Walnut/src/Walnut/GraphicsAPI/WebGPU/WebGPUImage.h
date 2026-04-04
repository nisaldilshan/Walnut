#pragma once

#include <stdint.h>
#include <stddef.h>

#include "WebGPUGraphics.h"
#include <Walnut/ImageFormat.h>

namespace GraphicsAPI
{
    class WebGPUImage
    {
    public:
        WebGPUImage() = default;
        ~WebGPUImage() = default;

        void CreateImage(Walnut::ImageFormat imageFormat, uint32_t width, uint32_t height);
        void CreateImageView();
        void UploadToBuffer(const void* data, size_t uploadSize);
        void CreateSampler();
        void CreateDescriptorSet();
        uint64_t GetHandleForImGui() const;
        wgpu::BindGroup GetBindGroup() const;
        wgpu::BindGroupLayout GetBindGroupLayout() const;
        bool ImageAvailable();
        void ResourceFree();
    private:
        wgpu::Texture m_texture = nullptr;
        wgpu::TextureView m_textureView = nullptr;
        wgpu::TextureFormat m_textureFormat = wgpu::TextureFormat::Undefined;
        uint32_t m_width, m_height;
        uint32_t m_DataFormat;
        void* m_imageBuffer;
        wgpu::Sampler m_sampler;
        wgpu::BindGroupLayout m_bindGroupLayout;
        wgpu::BindGroup m_bindGroup;
    };
}