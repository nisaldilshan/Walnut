#pragma once

#include "VulkanGraphics.h"
#include <Walnut/ImageFormat.h>

namespace GraphicsAPI
{
    class VulkanImage
    {
    public:
        VulkanImage() = default;
        ~VulkanImage() = default;

        size_t CreateUploadBuffer(size_t upload_size);
        void CreateImage(Walnut::ImageFormat imageFormat, uint32_t width, uint32_t height);
        void CreateImageView();
        void CopyToImage(VkCommandBuffer command_buffer, uint32_t width, uint32_t height);
        void UploadToBuffer(const void* data, size_t uploadSize, size_t alignedSize);
        void CreateSampler();
        void CreateDescriptorSet();
        ImTextureID GetDescriptorSet();
        bool ImageAvailable();
        void ResourceFree();
        VkBuffer GetStagingBuffer();

    private:
        VkImage m_Image;
        uint32_t m_width;
        uint32_t m_height;
        VkFormat m_imageFormat;
        VkImageView m_ImageView;
        VkSampler m_Sampler;
        VkDeviceMemory m_Memory;
        VkDescriptorSet m_DescriptorSet;
        VkBuffer m_StagingBuffer;
        VkDeviceMemory m_StagingBufferMemory;
    };
}