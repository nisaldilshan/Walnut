#pragma once

#include <stdint.h>
#include <stddef.h>

#include "OpenGLGraphics.h"

namespace GraphicsAPI
{
    class OpenGLImage
    {
        typedef int VkFormat;
        typedef int VkCommandBuffer;
    public:
        OpenGLImage() = default;
        ~OpenGLImage() = default;

        size_t CreateUploadBuffer(size_t upload_size);
        void CreateImage(VkFormat vulkanFormat, uint32_t width, uint32_t height);
        void CreateImageView(VkFormat vulkanFormat);
        void CopyToImage(VkCommandBuffer command_buffer, uint32_t width, uint32_t height);
        void UploadToBuffer(const void* data, size_t uploadSize, size_t alignedSize);
        void CreateSampler();
        void CreateDescriptorSet();
        ImTextureID GetDescriptorSet();
        bool ImageAvailable();
        void ResourceFree();
    private:
        GLuint m_texture;
        uint32_t m_Width, m_Height;
        uint32_t m_DataFormat;
        void* m_imageBuffer;
    };
}