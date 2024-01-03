#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../../Walnut/src/Walnut/GraphicsAPI/WebGPU/WebGPUGraphics.h"

#include <Walnut/GLM/GLM.h>

namespace GraphicsAPI
{
    class WebGPUCompute
    {
    public:
        WebGPUCompute() = default;
        ~WebGPUCompute() = default;

        void BeginComputePass();
        void EndComputePass();
        
    private:
        wgpu::CommandEncoder m_commandEncoder = nullptr;
        wgpu::ComputePassEncoder m_computePass = nullptr;
    };
}