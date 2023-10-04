#include "WebGPUImage.h"

#include "../../ImageFormat.h"

namespace Walnut
{
namespace Utils
{
    typedef int VkFormat;
    VkFormat WalnutFormatToVulkanFormat(ImageFormat format)
    {
        return (VkFormat)0;
    }
}

}

namespace GraphicsAPI
{

size_t WebGPUImage::CreateUploadBuffer(size_t upload_size)
{
    return size_t();
}

void WebGPUImage::CreateImage(int vulkanFormat, uint32_t width, uint32_t height)
{
}

void WebGPUImage::CreateImageView(int vulkanFormat)
{
}

void *WebGPUImage::GetDescriptorSet()
{
    return nullptr;
}

bool WebGPUImage::ImageAvailable()
{
    return false;
}

void WebGPUImage::ResourceFree()
{
}

typedef int VkBuffer;
VkBuffer WebGPUImage::GetStagingBuffer()
{
    return VkBuffer();
}

void WebGPUImage::UploadToBuffer(const void *data, size_t uploadSize, size_t alignedSize)
{
}

void WebGPUImage::CreateSampler()
{
}

void WebGPUImage::CreateDescriptorSet()
{
}
}
