#include "WebGPUCompute.h"

namespace GraphicsAPI
{

WebGPUCompute::~WebGPUCompute()
{
    
}

void WebGPUCompute::CreateBindGroup(const std::vector<wgpu::BindGroupLayoutEntry>& bindGroupLayoutEntries)
{
    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = (uint32_t)bindGroupLayoutEntries.size();
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    m_bindGroupLayout = WebGPU::GetDevice().createBindGroupLayout(bindGroupLayoutDesc);
}

void WebGPUCompute::CreateShaders(const char *shaderSource)
{
    std::cout << "Creating shader module..." << std::endl;

    wgpu::ShaderModuleDescriptor shaderDesc;
#ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
#endif

    wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
    // Set the chained struct's header
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource;
    // Connect the chain
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    m_shaderModule = WebGPU::GetDevice().createShaderModule(shaderDesc);

    std::cout << "Shader module: " << m_shaderModule << std::endl;
}

void WebGPUCompute::CreatePipeline()
{
    assert(m_bindGroupLayout);
    assert(m_shaderModule);

    // Create compute pipeline layout
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;
    auto pipelineLayout = WebGPU::GetDevice().createPipelineLayout(pipelineLayoutDesc);

    // Create compute pipeline
    wgpu::ComputePipelineDescriptor computePipelineDesc;
    computePipelineDesc.compute.constantCount = 0;
    computePipelineDesc.compute.constants = nullptr;
    computePipelineDesc.compute.entryPoint = "computeStuff";
    computePipelineDesc.compute.module = m_shaderModule;
    computePipelineDesc.layout = pipelineLayout;
    m_pipeline = WebGPU::GetDevice().createComputePipeline(computePipelineDesc);
}

void WebGPUCompute::BeginComputePass()
{
    wgpu::CommandEncoderDescriptor commandEncoderDesc;
    commandEncoderDesc.label = "Compute Command Encoder";
    m_commandEncoder = WebGPU::GetDevice().createCommandEncoder(commandEncoderDesc);
    

    wgpu::ComputePassDescriptor computePassDesc;
    computePassDesc.timestampWriteCount = 0;
    computePassDesc.timestampWrites = nullptr;

    m_computePass = m_commandEncoder.beginComputePass(computePassDesc);
}

void WebGPUCompute::EndComputePass()
{
    m_computePass.end();

    wgpu::CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = "Compute Command Buffer";
    wgpu::CommandBuffer commands = m_commandEncoder.finish(cmdBufferDescriptor);
    GraphicsAPI::WebGPU::GetQueue().submit(commands);

    m_commandEncoder.release();
}


}

