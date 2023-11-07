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
    m_rendererBackend->CreatePipeline();
}

void Renderer2D::SetShader(const char *shaderSource)
{
    m_rendererBackend->CreateShaders(shaderSource);
}

void Renderer2D::SetStandaloneShader(const char *shaderSource, uint32_t vertexShaderCallCount)
{
    m_rendererBackend->CreateStandaloneShader(shaderSource, vertexShaderCallCount);
}

void Renderer2D::SetBufferData(const std::vector<float>& bufferData, wgpu::VertexBufferLayout bufferLayout)
{
    m_rendererBackend->CreateBuffer("vertex", bufferData, bufferLayout);
}

void *Renderer2D::GetDescriptorSet()
{
    return m_rendererBackend->GetDescriptorSet();
}

void Renderer2D::Render()
{
    m_rendererBackend->Render();
}
