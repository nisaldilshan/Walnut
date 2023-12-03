#pragma once

#include <memory>
#include <stdint.h>
#include <Walnut/ImageFormat.h>

#include "../../Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h"

#include "Uniform.h"

namespace GraphicsAPI
{
#if (RENDERER_BACKEND == 1)
class OpenGLImage;
typedef OpenGLImage ImageType;
#elif (RENDERER_BACKEND == 2)
class VulkanImage;
typedef VulkanImage ImageType;
#elif (RENDERER_BACKEND == 3)
class WebGPURenderer3D;
typedef WebGPURenderer3D RendererType;
#else
#endif
}

class Renderer3D
{
public:
    Renderer3D(uint32_t width, uint32_t height, Walnut::ImageFormat format);
    ~Renderer3D();

    void Init();
    void SetShader(const char* shaderSource);
    void SetStandaloneShader(const char* shaderSource, uint32_t vertexShaderCallCount);
    void SetVertexBufferData(const std::vector<float>& bufferData, wgpu::VertexBufferLayout bufferLayout);
    void SetIndexBufferData(const std::vector<uint16_t>& bufferData);
    void SetBindGroupLayoutEntry(wgpu::BindGroupLayoutEntry bindGroupLayoutEntry);
    void SetSizeOfUniform(uint32_t sizeOfUniform);
    void CreateUniformBuffer(size_t dynamicOffsetCount);
    void SetUniformBufferData(const void* bufferData, uint32_t uniformIndex);
    void* GetDescriptorSet();
    uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }
    void SimpleRender();
    void Render();
    void RenderIndexed(uint32_t uniformIndex);
    void BeginRenderPass();
    void EndRenderPass();

private:
    uint32_t m_Width = 0, m_Height = 0;
    std::unique_ptr<GraphicsAPI::WebGPURenderer3D> m_rendererBackend;
    const char* m_shaderSource = nullptr;
};

