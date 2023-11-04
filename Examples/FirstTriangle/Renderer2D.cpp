#include "Renderer2D.h"

#include "WebGPURenderer2D.h"

Renderer2D::Renderer2D(uint32_t width, uint32_t height, Walnut::ImageFormat format)
    : m_Width(width)
    , m_Height(height)
    , m_renderer(std::make_unique<GraphicsAPI::WebGPURenderer2D>())
{
    m_renderer->CreateTextureToRenderInto(m_Width, m_Height);
}

Renderer2D::~Renderer2D()
{
}

void Renderer2D::Init()
{
    m_renderer->CreateShaders();
    m_renderer->CreatePipeline();
}

void *Renderer2D::GetDescriptorSet()
{
    return m_renderer->GetDescriptorSet();
}

void Renderer2D::Render()
{
    m_renderer->Render();
}
