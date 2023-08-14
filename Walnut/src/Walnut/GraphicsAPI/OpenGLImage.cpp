#include "OpenGLImage.h"

size_t GraphicsAPI::OpenGLImage::CreateUploadBuffer(size_t upload_size)
{
    return size_t();
}

void GraphicsAPI::OpenGLImage::CreateImage(VkFormat vulkanFormat, uint32_t width, uint32_t height)
{
}

void GraphicsAPI::OpenGLImage::CreateImageView(VkFormat vulkanFormat)
{
}

void GraphicsAPI::OpenGLImage::UploadToBuffer(const void *data, size_t uploadSize, size_t alignedSize)
{
}

void GraphicsAPI::OpenGLImage::CreateSampler()
{
}

void GraphicsAPI::OpenGLImage::CreateDescriptorSet()
{
}

VkDescriptorSet GraphicsAPI::OpenGLImage::GetDescriptorSet()
{
    return VkDescriptorSet();
}

bool GraphicsAPI::OpenGLImage::ImageAvailable()
{
    return false;
}

void GraphicsAPI::OpenGLImage::ResourceFree()
{
}
