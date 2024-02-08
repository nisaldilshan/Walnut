#include "Renderer3D.h"

#include "WebGPURenderer3D.h"

Renderer3D::Renderer3D()
    : m_Width(0)
    , m_Height(0)
    , m_rendererBackend(std::make_unique<GraphicsAPI::WebGPURenderer3D>())
{}

Renderer3D::~Renderer3D()
{}

void Renderer3D::OnResize(uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;
    m_rendererBackend->CreateTextureToRenderInto(m_Width, m_Height);
}

void Renderer3D::Init()
{
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

void Renderer3D::SetVertexBufferData(const void* bufferData, uint32_t bufferLength, wgpu::VertexBufferLayout bufferLayout)
{
    m_rendererBackend->CreateVertexBuffer(bufferData, bufferLength, bufferLayout);
}

void Renderer3D::SetIndexBufferData(const std::vector<uint16_t>& bufferData)
{
    m_rendererBackend->CreateIndexBuffer(bufferData);
}

void Renderer3D::CreateBindGroup(const std::vector<wgpu::BindGroupLayoutEntry>& bindGroupLayoutEntries)
{
    m_rendererBackend->CreateBindGroup(bindGroupLayoutEntries);
}

void Renderer3D::CreateTexture(uint32_t width, uint32_t height, const void* textureData, uint32_t mipMapLevelCount)
{
    m_rendererBackend->CreateTexture(width, height, textureData, mipMapLevelCount);
}

void Renderer3D::CreateTextureSampler()
{
    m_rendererBackend->CreateTextureSampler();
}

void Renderer3D::SetClearColor(glm::vec4 clearColor)
{
    m_rendererBackend->SetClearColor(clearColor);
}

void Renderer3D::CreateUniformBuffer(size_t bufferLength, UniformBuf::UniformType type, uint32_t sizeOfUniform, uint32_t bindingIndex)
{
    m_rendererBackend->CreateUniformBuffer(bufferLength, type, sizeOfUniform, bindingIndex);
}

void Renderer3D::SetUniformBufferData(UniformBuf::UniformType type, const void* bufferData, uint32_t uniformIndex)
{
    m_rendererBackend->SetUniformData(type, bufferData, uniformIndex);
}

void* Renderer3D::GetDescriptorSet()
{
    return m_rendererBackend->GetDescriptorSet();
}

void Renderer3D::SimpleRender()
{
    m_rendererBackend->SimpleRender();
}

void Renderer3D::Render(uint32_t uniformIndex)
{
    m_rendererBackend->Render(uniformIndex);
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
