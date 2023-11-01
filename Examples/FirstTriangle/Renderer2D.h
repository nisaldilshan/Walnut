#pragma once

#include <Walnut//ImageFormat.h>

class Renderer2D
{
public:
    Renderer2D(uint32_t width, uint32_t height, Walnut::ImageFormat format);
    ~Renderer2D();

    void* GetDescriptorSet();
    uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }
    void Render();
private:
    uint32_t m_Width = 0, m_Height = 0;
};

Renderer2D::Renderer2D(uint32_t width, uint32_t height, Walnut::ImageFormat format)
{
}

Renderer2D::~Renderer2D()
{
}

inline void *Renderer2D::GetDescriptorSet()
{
    return nullptr;
}

inline void Renderer2D::Render()
{
}
