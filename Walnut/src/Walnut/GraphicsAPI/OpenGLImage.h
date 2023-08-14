#pragma once

#include <stdint.h>
#include <stddef.h>

namespace GraphicsAPI
{

    typedef int VkFormat;
    typedef int VkCommandBuffer;
    typedef int VkDescriptorSet;
    class OpenGLImage
    {
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
        VkDescriptorSet GetDescriptorSet();
        bool ImageAvailable();
        void ResourceFree();
    private:
        // some
    };
}