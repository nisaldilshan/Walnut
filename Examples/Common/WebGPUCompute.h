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
        ~WebGPUCompute();

        void CreateBindGroup(const std::vector<wgpu::BindGroupLayoutEntry>& bindGroupLayoutEntries);
        void CreateShaders(const char *shaderSource);
        void CreatePipeline();
        void BeginComputePass();
        void EndComputePass();
    private:
        wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
        wgpu::ShaderModule m_shaderModule = nullptr;
        wgpu::ComputePipeline m_pipeline = nullptr;
        wgpu::CommandEncoder m_commandEncoder = nullptr;
        wgpu::ComputePassEncoder m_computePass = nullptr;
    };
}