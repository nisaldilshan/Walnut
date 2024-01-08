#include "Compute.h"

#include "WebGPUCompute.h"

Compute::Compute()
    : m_computeBackend(std::make_unique<GraphicsAPI::WebGPUCompute>())
{
}

Compute::~Compute()
{
}

void Compute::SetShader(const char *shaderSource)
{
    m_computeBackend->CreateShaders(shaderSource);
}

void Compute::CreateBindGroup(const std::vector<wgpu::BindGroupLayoutEntry>& bindGroupLayoutEntries)
{
    m_computeBackend->CreateBindGroup(bindGroupLayoutEntries);
}

void Compute::CreatePipeline()
{
    m_computeBackend->CreatePipeline();
}

void Compute::CreateBuffer(const uint32_t bufferLength, ComputeBuf::BufferType type)
{
    m_computeBackend->CreateBuffer(bufferLength, type);
}

void Compute::BeginComputePass()
{
    m_computeBackend->BeginComputePass();
}

void Compute::DoCompute(const void* bufferData, const uint32_t bufferLength)
{
    m_computeBackend->Compute(bufferData, bufferLength);
}

void Compute::EndComputePass()
{
    m_computeBackend->EndComputePass();
}
