#include "Renderer2D.h"

#include "WebGPURenderer2D.h"

Renderer2D::Renderer2D(uint32_t width, uint32_t height, Walnut::ImageFormat format)
    : m_Width(width)
    , m_Height(height)
    , m_rendererBackend(std::make_unique<GraphicsAPI::WebGPURenderer2D>())
{
    m_rendererBackend->CreateTextureToRenderInto(m_Width, m_Height);
}

Renderer2D::~Renderer2D()
{
}

void Renderer2D::Init()
{
    m_rendererBackend->CreateBindGroup();
    m_rendererBackend->CreatePipeline();
}

void Renderer2D::SetShader(const char* shaderSource)
{
    m_rendererBackend->CreateShaders(shaderSource);
}

void Renderer2D::SetStandaloneShader(const char* shaderSource, uint32_t vertexShaderCallCount)
{
    m_rendererBackend->CreateStandaloneShader(shaderSource, vertexShaderCallCount);
}

void Renderer2D::SetVertexBufferData(const std::vector<float>& bufferData, wgpu::VertexBufferLayout bufferLayout)
{
    m_rendererBackend->CreateVertexBuffer(bufferData, bufferLayout);
}

void Renderer2D::SetIndexBufferData(const std::vector<uint16_t>& bufferData)
{
    m_rendererBackend->CreateIndexBuffer(bufferData);
}

void Renderer2D::SetBindGroupLayoutEntry(wgpu::BindGroupLayoutEntry bindGroupLayoutEntry)
{
    m_rendererBackend->SetBindGroupLayoutEntry(bindGroupLayoutEntry);
}

void Renderer2D::CreateUniformBuffer()
{
    m_rendererBackend->CreateUniformBuffer();
}

void Renderer2D::SetUniformBufferData(const MyUniforms& bufferData, uint32_t uniformIndex)
{
    m_rendererBackend->SetUniformData(bufferData, uniformIndex);
}

void* Renderer2D::GetDescriptorSet()
{
    return m_rendererBackend->GetDescriptorSet();
}

void Renderer2D::Render()
{
    m_rendererBackend->RenderIndexed();
}
