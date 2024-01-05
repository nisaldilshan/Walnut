#include "Compute.h"

#include "WebGPUCompute.h"

Compute::Compute()
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

void Compute::CreateBuffer(const void* bufferData, uint32_t bufferLength, ComputeBuf::BufferType type)
{
    m_computeBackend->CreateBuffer(bufferData, bufferLength, type);
}

void Compute::BeginComputePass()
{
    m_computeBackend->BeginComputePass();
}

void Compute::DoCompute()
{
    m_computeBackend->Compute();
}

void Compute::EndComputePass()
{
    m_computeBackend->EndComputePass();
}
