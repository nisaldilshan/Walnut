#pragma once

#include <stdint.h>
#include <stddef.h>

namespace GraphicsAPI
{
    class WebGPUImage
    {
        typedef int VkCommandBuffer;
        typedef int VkBuffer;
    public:
        WebGPUImage() = default;
        ~WebGPUImage() = default;

        size_t CreateUploadBuffer(size_t upload_size);
        void CreateImage(int vulkanFormat, uint32_t width, uint32_t height);
        void CreateImageView(int vulkanFormat);
        void CopyToImage(VkCommandBuffer command_buffer, uint32_t width, uint32_t height);
        void UploadToBuffer(const void* data, size_t uploadSize, size_t alignedSize);
        void CreateSampler();
        void CreateDescriptorSet();
        void* GetDescriptorSet();
        bool ImageAvailable();
        void ResourceFree();
        VkBuffer GetStagingBuffer();
    private:
        int m_texture;
        uint32_t m_Width, m_Height;
        uint32_t m_DataFormat;
        void* m_imageBuffer;
    };
}