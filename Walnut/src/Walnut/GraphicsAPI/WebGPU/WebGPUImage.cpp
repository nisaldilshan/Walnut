#include "WebGPUImage.h"
#include <array>

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
        default:
            assert(false);
            return wgpu::TextureFormat::Undefined;
        }
    }
}

}

namespace GraphicsAPI
{

void WebGPUImage::CreateImage(Walnut::ImageFormat imageFormat, uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    m_textureFormat = Walnut::Utils::WalnutFormatToWebGPUFormat(imageFormat);
    
    wgpu::TextureDescriptor tex_desc = {};
    tex_desc.label = wgpu::StringView("Dear ImGui Font Texture");
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

uint64_t WebGPUImage::GetHandleForImGui() const
{
    return reinterpret_cast<uint64_t>((void*)m_textureView);
}

wgpu::BindGroup WebGPUImage::GetBindGroup() const 
{
    return m_bindGroup; 
}

wgpu::BindGroupLayout WebGPUImage::GetBindGroupLayout() const
{
    return m_bindGroupLayout;
}

bool WebGPUImage::ImageAvailable()
{
    return true;
}

void WebGPUImage::ResourceFree()
{
}

void WebGPUImage::UploadToBuffer(const void *data, size_t uploadSize)
{
    wgpu::TexelCopyTextureInfo dst_view = {};
    dst_view.texture = m_texture;
    dst_view.mipLevel = 0;
    dst_view.origin = { 0, 0, 0 };
    dst_view.aspect = WGPUTextureAspect_All;
    wgpu::TexelCopyBufferLayout layout = {};
    layout.offset = 0;

    int size_pp = 4; // size per pixel
    layout.bytesPerRow = m_width * size_pp;
    layout.rowsPerImage = m_height;
    WGPUExtent3D size = { (uint32_t)m_width, (uint32_t)m_height, 1 };
    WebGPU::GetQueue().writeTexture(dst_view, data, (uint32_t)(m_width * size_pp * m_height), layout, size);
}

void WebGPUImage::CreateSampler()
{
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = wgpu::AddressMode::Repeat;
    samplerDesc.addressModeV = wgpu::AddressMode::Repeat;
    samplerDesc.addressModeW = wgpu::AddressMode::Repeat;
    samplerDesc.magFilter = wgpu::FilterMode::Linear;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 8.0f;
    samplerDesc.compare = wgpu::CompareFunction::Undefined;
    samplerDesc.maxAnisotropy = 1;
    m_sampler = GraphicsAPI::WebGPU::GetDevice().createSampler(samplerDesc);
}

void WebGPUImage::CreateDescriptorSet()
{
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    entries.resize(2);
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Fragment;
    entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[0].texture.viewDimension = wgpu::TextureViewDimension::_2D;
    entries[0].texture.multisampled = false;
    entries[1].binding = 1;
    entries[1].visibility = wgpu::ShaderStage::Fragment;
    entries[1].sampler.type = wgpu::SamplerBindingType::Filtering;
    // Create a bind group layout
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
	bindGroupLayoutDesc.entryCount = entries.size();
	bindGroupLayoutDesc.entries = entries.data();
    m_bindGroupLayout = WebGPU::GetDevice().createBindGroupLayout(bindGroupLayoutDesc);
    assert(m_bindGroupLayout);

    std::array<wgpu::BindGroupEntry, 2> bindings;
    bindings[0].binding = 0;
    bindings[0].textureView = m_textureView;
    bindings[1].binding = 1;
    bindings[1].sampler = m_sampler;

    wgpu::BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.layout = m_bindGroupLayout;
    bindGroupDesc.entryCount = bindings.size();
    bindGroupDesc.entries = bindings.data();
    m_bindGroup = GraphicsAPI::WebGPU::GetDevice().createBindGroup(bindGroupDesc);
}

} // namespace GraphicsAPI