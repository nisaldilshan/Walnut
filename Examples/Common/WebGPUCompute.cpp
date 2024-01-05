#include "WebGPUCompute.h"

namespace GraphicsAPI
{

WebGPUCompute::~WebGPUCompute()
{
    
}

void WebGPUCompute::CreateBindGroup(const std::vector<wgpu::BindGroupLayoutEntry>& bindGroupLayoutEntries)
{
    std::cout << "Creating compute bind group..." << std::endl;
    // Create a bind group layout using a vector of layout entries
    auto bindGroupLayoutEntryCount = (uint32_t)bindGroupLayoutEntries.size();
    assert(bindGroupLayoutEntryCount > 0);

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = (uint32_t)bindGroupLayoutEntries.size();
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    m_bindGroupLayout = WebGPU::GetDevice().createBindGroupLayout(bindGroupLayoutDesc);

    
    std::vector<wgpu::BindGroupEntry> bindings;
    bindings.resize(bindGroupLayoutEntryCount);

    wgpu::BindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = m_bindGroupLayout;
	bindGroupDesc.entryCount = (uint32_t)bindings.size();
	bindGroupDesc.entries = (WGPUBindGroupEntry*)bindings.data();
    m_bindGroup = WebGPU::GetDevice().createBindGroup(bindGroupDesc);

    std::cout << "Compute bind group: " << m_bindGroup << std::endl;
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
    std::cout << "Creating compute pipeline..." << std::endl;
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

    std::cout << "Compute pipeline: " << m_pipeline << std::endl;
}

void WebGPUCompute::CreateBuffer(uint32_t bufferLength, ComputeBuf::BufferType type)
{
    std::cout << "Creating buffer..." << std::endl;
    std::cout << "Vertex buffer: " << std::endl;
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

void WebGPUCompute::Compute(const void *bufferData, uint32_t bufferLength)
{
    m_computePass.setPipeline(m_pipeline);
    m_computePass.setBindGroup(0, m_bindGroup, 0, nullptr);
    constexpr uint32_t workgroupCount = 2;
	m_computePass.dispatchWorkgroups(workgroupCount, 1, 1);
}

void WebGPUCompute::EndComputePass()
{
    m_computePass.end();

    // Have to copy buffers before encoder.finish
	//m_commandEncoder.copyBufferToBuffer(m_outputBuffer, 0, m_mapBuffer, 0, m_bufferSize);

    wgpu::CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = "Compute Command Buffer";
    wgpu::CommandBuffer commands = m_commandEncoder.finish(cmdBufferDescriptor);
    GraphicsAPI::WebGPU::GetQueue().submit(commands);

    //m_commandEncoder.release();
}


}

