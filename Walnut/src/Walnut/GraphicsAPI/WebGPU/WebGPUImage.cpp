#include "WebGPUImage.h"

namespace Walnut
{
namespace Utils
{
    wgpu::TextureFormat WalnutFormatToWebGPUFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGBA:
            return wgpu::TextureFormat::RGBA8Unorm;
        case ImageFormat::RGBA32F:
            return wgpu::TextureFormat::RGBA32Uint;
        case ImageFormat::None:
            assert(false);
            return wgpu::TextureFormat::Undefined;
        }
    }
}

}

namespace GraphicsAPI
{

size_t WebGPUImage::CreateUploadBuffer(size_t upload_size)
{
    return 1;
}

void WebGPUImage::CreateImage(Walnut::ImageFormat imageFormat, uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    m_textureFormat = Walnut::Utils::WalnutFormatToWebGPUFormat(imageFormat);
    
    wgpu::TextureDescriptor tex_desc = {};
    tex_desc.label = "Dear ImGui Font Texture";
    tex_desc.dimension = WGPUTextureDimension_2D;
    tex_desc.size.width = width;
    tex_desc.size.height = height;
    tex_desc.size.depthOrArrayLayers = 1;
    tex_desc.sampleCount = 1;
    tex_desc.format = m_textureFormat;
    tex_desc.mipLevelCount = 1;
    tex_desc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
    m_texture = WebGPU::GetDevice().createTexture(tex_desc);
}

void WebGPUImage::CreateImageView()
{
    wgpu::TextureViewDescriptor tex_view_desc = {};
    tex_view_desc.format = m_textureFormat;
    tex_view_desc.dimension = WGPUTextureViewDimension_2D;
    tex_view_desc.baseMipLevel = 0;
    tex_view_desc.mipLevelCount = 1;
    tex_view_desc.baseArrayLayer = 0;
    tex_view_desc.arrayLayerCount = 1;
    tex_view_desc.aspect = WGPUTextureAspect_All;
    m_textureView = m_texture.createView(tex_view_desc);
}

ImTextureID WebGPUImage::GetDescriptorSet()
{
    return m_textureView;
}

bool WebGPUImage::ImageAvailable()
{
    return true;
}

void WebGPUImage::ResourceFree()
{
}

WebGPUImage::VkBuffer WebGPUImage::GetStagingBuffer()
{
    return 0;
}

void WebGPUImage::UploadToBuffer(const void *data, size_t uploadSize, size_t alignedSize)
{
    wgpu::ImageCopyTexture dst_view = {};
    dst_view.texture = m_texture;
    dst_view.mipLevel = 0;
    dst_view.origin = { 0, 0, 0 };
    dst_view.aspect = WGPUTextureAspect_All;
    wgpu::TextureDataLayout layout = {};
    layout.offset = 0;

    int size_pp = 4; // size per pixel
    layout.bytesPerRow = m_width * size_pp;
    layout.rowsPerImage = m_height;
    WGPUExtent3D size = { (uint32_t)m_width, (uint32_t)m_height, 1 };
    WebGPU::GetQueue().writeTexture(dst_view, data, (uint32_t)(m_width * size_pp * m_height), layout, size);
}

void WebGPUImage::CreateSampler()
{
}

void WebGPUImage::CreateDescriptorSet()
{
}
}
