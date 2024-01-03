#include "Compute.h"

#include "WebGPUCompute.h"

Compute::Compute()
{
}

Compute::~Compute()
{
}

void Compute::Init()
{
}

void Compute::BeginComputePass()
{
    m_computeBackend->BeginComputePass();
}

void Compute::EndComputePass()
{
}
