#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef __EMSCRIPTEN__
#include <glad/glad.h>
#endif
#include "OpenGLGraphics.h"
#include <Walnut/ImageFormat.h>

namespace GraphicsAPI
{
    class OpenGLImage
    {
        typedef int VkCommandBuffer;
        typedef int VkBuffer;
    public:
        OpenGLImage() = default;
        ~OpenGLImage() = default;

        size_t CreateUploadBuffer(size_t upload_size);
        void CreateImage(Walnut::ImageFormat imageFormat, uint32_t width, uint32_t height);
        void CreateImageView();
        void CopyToImage(VkCommandBuffer command_buffer, uint32_t width, uint32_t height);
        void UploadToBuffer(const void* data, size_t uploadSize, size_t alignedSize);
        void CreateSampler();
        void CreateDescriptorSet();
        void* GetDescriptorSet();
        bool ImageAvailable();
        void ResourceFree();
        VkBuffer GetStagingBuffer();
    private:
        GLuint m_texture;
        uint32_t m_Width, m_Height;
        uint32_t m_DataFormat;
        void* m_imageBuffer;
    };
}