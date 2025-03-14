#include "VulkanImage.h"

#include <imgui_impl_vulkan.h>

#include "../../ImageFormat.h"

namespace Walnut
{
namespace Utils
{
    VkFormat WalnutFormatToVulkanFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ImageFormat::None: assert(false);
        }
        return (VkFormat)0;
    }
}

}

namespace GraphicsAPI
{
    
VulkanImage::VulkanImage()
{
    auto queueFamilyIndices = Vulkan::FindQueueFamilies();
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    auto err = vkCreateCommandPool(Vulkan::GetDevice(), &poolInfo, nullptr, &m_commandPool);
    Vulkan::check_vk_result(err);
    
    VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
    cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocateInfo.commandPool = m_commandPool;
    cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocateInfo.commandBufferCount = 1;
    err = vkAllocateCommandBuffers(Vulkan::GetDevice(), &cmdBufAllocateInfo, &m_commandBuffer);
    Vulkan::check_vk_result(err);
}

VulkanImage::~VulkanImage()
{
    if (m_commandPool)
    {
        if (m_commandBuffer)
        {
            vkFreeCommandBuffers(Vulkan::GetDevice(), m_commandPool, 1, &m_commandBuffer);
        }

        vkDestroyCommandPool(Vulkan::GetDevice(), m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
}

size_t VulkanImage::CreateUploadBuffer(size_t upload_size)
{
    VkResult err;
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = upload_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vkCreateBuffer(Vulkan::GetDevice(), &buffer_info, nullptr, &m_StagingBuffer);
    Vulkan::check_vk_result(err);
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(Vulkan::GetDevice(), m_StagingBuffer, &req);
    size_t alignedSize = req.size;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = Utils::GetVulkanMemoryType(Vulkan::GetPhysicalDevice(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
    err = vkAllocateMemory(Vulkan::GetDevice(), &alloc_info, nullptr, &m_StagingBufferMemory);
    Vulkan::check_vk_result(err);
    err = vkBindBufferMemory(Vulkan::GetDevice(), m_StagingBuffer, m_StagingBufferMemory, 0);
    Vulkan::check_vk_result(err);
    return alignedSize;
}

void VulkanImage::CreateImage(Walnut::ImageFormat imageFormat, uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    m_imageFormat = Walnut::Utils::WalnutFormatToVulkanFormat(imageFormat);

    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = m_imageFormat;
    info.extent.width = m_width;
    info.extent.height = m_height;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkResult err = vkCreateImage(Vulkan::GetDevice(), &info, nullptr, &m_Image);
    Vulkan::check_vk_result(err);
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(Vulkan::GetDevice(), m_Image, &req);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = Utils::GetVulkanMemoryType(Vulkan::GetPhysicalDevice(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
    err = vkAllocateMemory(Vulkan::GetDevice(), &alloc_info, nullptr, &m_Memory);
    Vulkan::check_vk_result(err);
    err = vkBindImageMemory(Vulkan::GetDevice(), m_Image, m_Memory, 0);
    Vulkan::check_vk_result(err);
}

void VulkanImage::CreateImageView()
{
    VkResult err;
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = m_Image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = m_imageFormat;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.layerCount = 1;
    err = vkCreateImageView(Vulkan::GetDevice(), &info, nullptr, &m_ImageView);
    Vulkan::check_vk_result(err);
}

void VulkanImage::CopyToImage(VkCommandBuffer command_buffer, uint32_t width, uint32_t height)
{
    VkImageMemoryBarrier copy_barrier = {};
    copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier.image = m_Image;
    copy_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_barrier.subresourceRange.levelCount = 1;
    copy_barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &copy_barrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(command_buffer, m_StagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier use_barrier = {};
    use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier.image = m_Image;
    use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    use_barrier.subresourceRange.levelCount = 1;
    use_barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &use_barrier);
}

void VulkanImage::UploadToBuffer(const void* data, size_t uploadSize, size_t alignedSize)
{
    char* map = NULL;
    VkResult err = vkMapMemory(Vulkan::GetDevice(), m_StagingBufferMemory, 0, alignedSize, 0, (void**)(&map));
    Vulkan::check_vk_result(err);
    memcpy(map, data, uploadSize);
    VkMappedMemoryRange range[1] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = m_StagingBufferMemory;
    range[0].size = alignedSize;
    err = vkFlushMappedMemoryRanges(Vulkan::GetDevice(), 1, range);
    Vulkan::check_vk_result(err);
    vkUnmapMemory(Vulkan::GetDevice(), m_StagingBufferMemory);

    // Copy to Image
    {
        auto err = vkResetCommandBuffer(m_commandBuffer, 0);
        Vulkan::check_vk_result(err);
        
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(m_commandBuffer, &begin_info);
        Vulkan::check_vk_result(err);

        CopyToImage(m_commandBuffer, m_width, m_height);

        err = vkEndCommandBuffer(m_commandBuffer);
        Vulkan::check_vk_result(err);

        VkSubmitInfo end_info{};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &m_commandBuffer;
        GraphicsAPI::Vulkan::QueueSubmit(end_info);
    }
}

void VulkanImage::CreateSampler()
{
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.minLod = -1000;
    info.maxLod = 1000;
    info.maxAnisotropy = 1.0f;
    VkResult err = vkCreateSampler(Vulkan::GetDevice(), &info, nullptr, &m_Sampler);
    Vulkan::check_vk_result(err);
}

void VulkanImage::CreateDescriptorSet()
{
    m_DescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void* VulkanImage::GetDescriptorSet()
{
	return (ImTextureID)m_DescriptorSet;
}

bool VulkanImage::ImageAvailable()
{
    if (m_Image)
        return true;

    return false;
}

void VulkanImage::ResourceFree()
{
    auto func = [  sampler = m_Sampler, imageView = m_ImageView, image = m_Image,
                        memory = m_Memory, stagingBuffer = m_StagingBuffer, stagingBufferMemory = m_StagingBufferMemory]()
    {
        vkDestroySampler(Vulkan::GetDevice(), sampler, nullptr);
        vkDestroyImageView(Vulkan::GetDevice(), imageView, nullptr);
        vkDestroyImage(Vulkan::GetDevice(), image, nullptr);
        vkFreeMemory(Vulkan::GetDevice(), memory, nullptr);
        vkDestroyBuffer(Vulkan::GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(Vulkan::GetDevice(), stagingBufferMemory, nullptr); 
    };

    Vulkan::SubmitResourceFree(func);

    m_Sampler = 0;
    m_ImageView = 0;
    m_Image = 0;
    m_Memory = 0;
    m_StagingBuffer = 0;
    m_StagingBufferMemory = 0;
}

VkBuffer VulkanImage::GetStagingBuffer()
{
    return m_StagingBuffer;
}

} // namespace