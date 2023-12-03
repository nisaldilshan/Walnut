#include "Renderer3D.h"

#include "WebGPURenderer3D.h"

Renderer3D::Renderer3D(uint32_t width, uint32_t height, Walnut::ImageFormat format)
    : m_Width(width)
    , m_Height(height)
    , m_rendererBackend(std::make_unique<GraphicsAPI::WebGPURenderer3D>())
{
    m_rendererBackend->CreateTextureToRenderInto(m_Width, m_Height);
}

Renderer3D::~Renderer3D()
{
}

void Renderer3D::Init()
{
    m_rendererBackend->CreateBindGroup();
    m_rendererBackend->CreatePipeline();
    m_rendererBackend->CreateDepthTexture();
}

void Renderer3D::SetShader(const char* shaderSource)
{
    m_rendererBackend->CreateShaders(shaderSource);
}

void Renderer3D::SetStandaloneShader(const char* shaderSource, uint32_t vertexShaderCallCount)
{
    m_rendererBackend->CreateStandaloneShader(shaderSource, vertexShaderCallCount);
}

void Renderer3D::SetVertexBufferData(const std::vector<float>& bufferData, wgpu::VertexBufferLayout bufferLayout)
{
    m_rendererBackend->CreateVertexBuffer(bufferData, bufferLayout);
}

void Renderer3D::SetIndexBufferData(const std::vector<uint16_t>& bufferData)
{
    m_rendererBackend->CreateIndexBuffer(bufferData);
}

void Renderer3D::SetBindGroupLayoutEntry(wgpu::BindGroupLayoutEntry bindGroupLayoutEntry)
{
    m_rendererBackend->SetBindGroupLayoutEntry(bindGroupLayoutEntry);
}

void Renderer3D::SetSizeOfUniform(uint32_t sizeOfUniform)
{
    m_rendererBackend->SetSizeOfUniform(sizeOfUniform);
}

void Renderer3D::CreateUniformBuffer(size_t dynamicOffsetCount)
{
    m_rendererBackend->CreateUniformBuffer(dynamicOffsetCount);
}

void Renderer3D::SetUniformBufferData(const void* bufferData, uint32_t uniformIndex)
{
    m_rendererBackend->SetUniformData(bufferData, uniformIndex);
}

void* Renderer3D::GetDescriptorSet()
{
    return m_rendererBackend->GetDescriptorSet();
}

void Renderer3D::SimpleRender()
{
    m_rendererBackend->SimpleRender();
}

void Renderer3D::Render()
{
    m_rendererBackend->Render();
}

void Renderer3D::RenderIndexed(uint32_t uniformIndex)
{
    m_rendererBackend->RenderIndexed(uniformIndex);
}

void Renderer3D::BeginRenderPass()
{
    m_rendererBackend->BeginRenderPass();
}

void Renderer3D::EndRenderPass()
{
    m_rendererBackend->EndRenderPass();
}
