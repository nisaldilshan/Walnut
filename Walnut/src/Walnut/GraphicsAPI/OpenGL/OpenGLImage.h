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
    public:
        OpenGLImage() = default;
        ~OpenGLImage() = default;

        void CreateImage(Walnut::ImageFormat imageFormat, uint32_t width, uint32_t height);
        void CreateImageView();
        void UploadToBuffer(const void* data, size_t uploadSize);
        void CreateSampler();
        void CreateDescriptorSet();
        uint64_t GetHandleForImGui();
        bool ImageAvailable();
        void ResourceFree();
    private:
        GLuint m_texture;
        uint32_t m_Width, m_Height;
        uint32_t m_DataFormat;
        void* m_imageBuffer;
    };
}