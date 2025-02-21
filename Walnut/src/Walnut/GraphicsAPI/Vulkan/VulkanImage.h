#pragma once

#include "VulkanGraphics.h"
#include <Walnut/ImageFormat.h>

namespace GraphicsAPI
{
    class VulkanImage
    {
    public:
        VulkanImage();
        ~VulkanImage();
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
        VkImage m_Image = VK_NULL_HANDLE;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        VkFormat m_imageFormat = VK_FORMAT_UNDEFINED;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
        VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_StagingBufferMemory = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    };
}