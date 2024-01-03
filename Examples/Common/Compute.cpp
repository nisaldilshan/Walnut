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

void Compute::BeginComputePass()
{
    m_computeBackend->BeginComputePass();
}

void Compute::EndComputePass()
{
}
