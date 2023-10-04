#include "OpenGLImage.h"

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

size_t OpenGLImage::CreateUploadBuffer(size_t upload_size)
{
    return 1;
}

void OpenGLImage::CreateImage(int vulkanFormat, uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;
    m_DataFormat = GL_RGBA;

    glGenTextures(1, &m_texture); // glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
    
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Load up some initial data
    // glTexImage2D(GL_TEXTURE_2D, 0, m_DataFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, (GLvoid*)m_imageBuffer);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
}

void OpenGLImage::CreateImageView(int vulkanFormat)
{
}

void OpenGLImage::UploadToBuffer(const void *data, size_t uploadSize, size_t alignedSize)
{
    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)m_texture);
	// Load up the new data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)data);
}

void OpenGLImage::CreateSampler()
{
}

void OpenGLImage::CreateDescriptorSet()
{
}

ImTextureID OpenGLImage::GetDescriptorSet()
{
    return (ImTextureID)(intptr_t)m_texture;
}

bool OpenGLImage::ImageAvailable()
{
    return true;
}

void OpenGLImage::ResourceFree()
{
    glDeleteTextures(1, &m_texture);
}

OpenGLImage::VkBuffer OpenGLImage::GetStagingBuffer()
{
    return 0;
}

}
