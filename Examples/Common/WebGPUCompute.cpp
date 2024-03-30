#include "WebGPUCompute.h"

namespace GraphicsAPI
{

WebGPUCompute::~WebGPUCompute()
{
    if (m_inputBuffer)
    {
        m_inputBuffer.destroy();
	    m_inputBuffer.release();
        m_inputBuffer = nullptr;
    }
    
    if (m_outputBuffer)
    {
        m_outputBuffer.destroy();
	    m_outputBuffer.release();
        m_outputBuffer = nullptr;
    }
	
    if (m_mapBuffer)
    {
        m_mapBuffer.destroy();
	    m_mapBuffer.release();
        m_mapBuffer = nullptr;
    }
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

    // Input buffer
	bindings[0].binding = 0;
	bindings[0].buffer = m_inputBuffer;
	bindings[0].offset = 0;
	bindings[0].size = m_inputBuffer.getSize();

	// Output buffer
	bindings[1].binding = 1;
	bindings[1].buffer = m_outputBuffer;
	bindings[1].offset = 0;
	bindings[1].size = m_outputBuffer.getSize();

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
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.mappedAtCreation = false;
    bufferDesc.size = bufferLength;

    switch (type)
    {
    case ComputeBuf::BufferType::Input:
        std::cout << "Creating input buffer..." << std::endl;
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        m_inputBuffer = WebGPU::GetDevice().createBuffer(bufferDesc);
        std::cout << "input buffer: " << m_inputBuffer << std::endl;
        break;
    case ComputeBuf::BufferType::Output:
        std::cout << "Creating output buffer..." << std::endl;
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        m_outputBuffer = WebGPU::GetDevice().createBuffer(bufferDesc);
        std::cout << "output buffer: " << m_outputBuffer << std::endl;
        break;
    case ComputeBuf::BufferType::Map:
        std::cout << "Creating map buffer..." << std::endl;
        bufferDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
        m_mapBuffer = WebGPU::GetDevice().createBuffer(bufferDesc);
        std::cout << "map buffer: " << m_mapBuffer << std::endl;
        break;
    }
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
    WebGPU::GetQueue().writeBuffer(m_inputBuffer, 0, bufferData, bufferLength);

    m_computePass.setPipeline(m_pipeline);
    m_computePass.setBindGroup(0, m_bindGroup, 0, nullptr);
    constexpr uint32_t workgroupCount = 2;
	m_computePass.dispatchWorkgroups(workgroupCount, 1, 1);
}

void WebGPUCompute::EndComputePass()
{
    m_computePass.end();

    // Have to copy buffers before encoder.finish
    auto sizeOfMapBuffer = m_mapBuffer.getSize();
    std::cout << "sizeOfMapBuffer : " << sizeOfMapBuffer << std::endl;
    // Copy the memory from the output buffer that lies in the storage part of the
    // memory to the map buffer, which is in the "mappable" part of the memory.
	m_commandEncoder.copyBufferToBuffer(m_outputBuffer, 0, m_mapBuffer, 0, sizeOfMapBuffer);

    wgpu::CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = "Compute Command Buffer";
    wgpu::CommandBuffer commands = m_commandEncoder.finish(cmdBufferDescriptor);
    WebGPU::GetQueue().submit(commands);

    // Print output
	bool done = false;
	auto handle = m_mapBuffer.mapAsync(wgpu::MapMode::Read, 0, sizeOfMapBuffer, [&](wgpu::BufferMapAsyncStatus status) {
		if (status == wgpu::BufferMapAsyncStatus::Success) {
			const float* output = (const float*)m_mapBuffer.getConstMappedRange(0, sizeOfMapBuffer);
			for (int i = 0; i < sizeOfMapBuffer/sizeof(float); ++i) {
				std::cout << "output " << output[i] << std::endl;
			}
			m_mapBuffer.unmap();
		}
		done = true;
	});

	while (!done) {
		// Checks for ongoing asynchronous operations and call their callbacks if needed
#ifdef WEBGPU_BACKEND_WGPU
        queue.submit(0, nullptr);
#else
        WebGPU::GetInstance().processEvents();
#endif
	}

    m_commandEncoder.release();
}


}

