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
    wgpu::TextureDescriptor tex_desc = {};
    tex_desc.label = "Dear ImGui Font Texture";
    tex_desc.dimension = WGPUTextureDimension_2D;
    tex_desc.size.width = width;
    tex_desc.size.height = height;
    tex_desc.size.depthOrArrayLayers = 1;
    tex_desc.sampleCount = 1;
    tex_desc.format = WGPUTextureFormat_RGBA8Unorm;
    tex_desc.mipLevelCount = 1;
    tex_desc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
    auto device = WebGPU::GetDevice();
    m_texture = device.createTexture(tex_desc);
}

void WebGPUImage::CreateImageView(int vulkanFormat)
{
    wgpu::TextureViewDescriptor tex_view_desc = {};
    tex_view_desc.format = WGPUTextureFormat_RGBA8Unorm;
    tex_view_desc.dimension = WGPUTextureViewDimension_2D;
    tex_view_desc.baseMipLevel = 0;
    tex_view_desc.mipLevelCount = 1;
    tex_view_desc.baseArrayLayer = 0;
    tex_view_desc.arrayLayerCount = 1;
    tex_view_desc.aspect = WGPUTextureAspect_All;
    wgpu::TextureView textureView = m_texture.createView(tex_view_desc);
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
