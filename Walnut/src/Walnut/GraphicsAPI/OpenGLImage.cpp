#include "OpenGLImage.h"

namespace GraphicsAPI
{

size_t OpenGLImage::CreateUploadBuffer(size_t upload_size)
{
    return size_t();
}

void OpenGLImage::CreateImage(VkFormat vulkanFormat, uint32_t width, uint32_t height)
{
}

void OpenGLImage::CreateImageView(VkFormat vulkanFormat)
{
}

void OpenGLImage::UploadToBuffer(const void *data, size_t uploadSize, size_t alignedSize)
{
}

void OpenGLImage::CreateSampler()
{
}

void OpenGLImage::CreateDescriptorSet()
{
}

VkDescriptorSet OpenGLImage::GetDescriptorSet()
{
    return VkDescriptorSet();
}

bool OpenGLImage::ImageAvailable()
{
    return false;
}

void OpenGLImage::ResourceFree()
{
}

}

