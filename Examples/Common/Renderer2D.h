#pragma once

#include <memory>
#include <stdint.h>
#include <Walnut/ImageFormat.h>

#include "../../Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h"

namespace GraphicsAPI
{
#if (RENDERER_BACKEND == 1)
class OpenGLImage;
typedef OpenGLImage ImageType;
#elif (RENDERER_BACKEND == 2)
class VulkanImage;
typedef VulkanImage ImageType;
#elif (RENDERER_BACKEND == 3)
class WebGPURenderer2D;
typedef WebGPURenderer2D RendererType;
#else
#endif
}

class Renderer2D
{
public:
    Renderer2D(uint32_t width, uint32_t height, Walnut::ImageFormat format);
    ~Renderer2D();

    void Init();
    void SetShader(const char* shaderSource);
    void SetStandaloneShader(const char* shaderSource, uint32_t vertexShaderCallCount);
    void SetBufferData(const std::vector<float>& bufferData, wgpu::VertexBufferLayout bufferLayout);
    void* GetDescriptorSet();
    uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }
    void Render();
private:
    
    uint32_t m_Width = 0, m_Height = 0;
    std::unique_ptr<GraphicsAPI::WebGPURenderer2D> m_rendererBackend;
    const char* m_shaderSource = nullptr;
};

