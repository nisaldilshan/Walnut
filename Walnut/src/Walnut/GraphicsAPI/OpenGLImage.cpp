#include "OpenGLImage.h"

namespace GraphicsAPI
{

size_t OpenGLImage::CreateUploadBuffer(size_t upload_size)
{
    return 1;
}

void OpenGLImage::CreateImage(VkFormat vulkanFormat, uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;
    GLenum internalFormat = GL_RGBA8;
    m_DataFormat = GL_RGBA;
    // glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
    // glTextureStorage2D(m_texture, 1, internalFormat, m_Width, m_Height);

    // glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Load up some initial data
    glTexImage2D(GL_TEXTURE_2D, 0, m_DataFormat, 256, 256, 0, m_DataFormat, GL_UNSIGNED_BYTE, (GLvoid*)m_imageBuffer);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
}

void OpenGLImage::CreateImageView(VkFormat vulkanFormat)
{
}

void OpenGLImage::UploadToBuffer(const void *data, size_t uploadSize, size_t alignedSize)
{
    //glTextureSubImage2D(m_texture, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)m_texture);
		// Load up the new data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)data);
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
    return false;
}

void OpenGLImage::ResourceFree()
{
}

}

