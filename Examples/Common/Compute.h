#pragma once

#include <memory>
#include <stdint.h>

#include "../../Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h"

#include <Walnut/GLM/GLM.h>
#include "Buffer.h"

namespace GraphicsAPI
{
#if (RENDERER_BACKEND == 1)
static_assert(false);
#elif (RENDERER_BACKEND == 2)
static_assert(false);
#elif (RENDERER_BACKEND == 3)
class WebGPUCompute;
typedef WebGPUCompute ComputeType;
#else
#endif
}

class Compute
{
public:
    Compute();
    ~Compute();
    void SetShader(const char* shaderSource);
    void CreateBindGroup(const std::vector<wgpu::BindGroupLayoutEntry>& bindGroupLayoutEntries);
    void CreatePipeline();
    void CreateBuffer(const void* bufferData, uint32_t bufferLength, ComputeBuf::BufferType type);
    void BeginComputePass();
    void DoCompute();
    void EndComputePass();
private:
    std::unique_ptr<GraphicsAPI::ComputeType> m_computeBackend;
};

