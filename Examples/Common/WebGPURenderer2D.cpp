#include "WebGPURenderer2D.h"

namespace GraphicsAPI
{

void WebGPURenderer2D::CreateTextureToRenderInto(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    wgpu::TextureDescriptor tex_desc = {};
    tex_desc.label = "Renderer Final Texture";
    tex_desc.dimension = WGPUTextureDimension_2D;
    tex_desc.size.width = m_width;
    tex_desc.size.height = m_height;
    tex_desc.size.depthOrArrayLayers = 1;
    tex_desc.sampleCount = 1;
    tex_desc.format = WGPUTextureFormat_BGRA8Unorm;
    tex_desc.mipLevelCount = 1;
    tex_desc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment;
    //##
    tex_desc.viewFormatCount = 1;
    wgpu::TextureFormat tf = WebGPU::GetSwapChainFormat();
	tex_desc.viewFormats = (WGPUTextureFormat *)const_cast<wgpu::TextureFormat *>(&tf);
    //##
    wgpu::Texture texture = WebGPU::GetDevice().createTexture(tex_desc);

    wgpu::TextureViewDescriptor tex_view_desc = {};
    tex_view_desc.format = WGPUTextureFormat_BGRA8Unorm;
    tex_view_desc.dimension = WGPUTextureViewDimension_2D;
    tex_view_desc.baseMipLevel = 0;
    tex_view_desc.mipLevelCount = 1;
    tex_view_desc.baseArrayLayer = 0;
    tex_view_desc.arrayLayerCount = 1;
    tex_view_desc.aspect = WGPUTextureAspect_All;
    m_textureToRenderInto = texture.createView(tex_view_desc);
}

void WebGPURenderer2D::CreateShaders(const char* shaderSource)
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

void WebGPURenderer2D::CreateStandaloneShader(const char *shaderSource, uint32_t vertexShaderCallCount)
{
    CreateShaders(shaderSource);
    m_vertexCount = vertexShaderCallCount;
}

void WebGPURenderer2D::CreatePipeline()
{
    std::cout << "Creating render pipeline..." << std::endl;

    wgpu::RenderPipelineDescriptor pipelineDesc;

    // Vertex fetch
    if (m_vertexBufferSize > 0)
    {
        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &m_vertexBufferLayout;
    }
    else
    {
        pipelineDesc.vertex.bufferCount = 0;
        pipelineDesc.vertex.buffers = nullptr;
    }
    // Vertex shader
    pipelineDesc.vertex.module = m_shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

    // Primitive assembly and rasterization
	// Each sequence of 3 vertices is considered as a triangle
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	// We'll see later how to specify the order in which vertices should be
	// connected. When not specified, vertices are considered sequentially.
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	// The face orientation is defined by assuming that when looking
	// from the front of the face, its corner vertices are enumerated
	// in the counter-clockwise (CCW) order.
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	// But the face orientation does not matter much because we do not
	// cull (i.e. "hide") the faces pointing away from us (which is often
	// used for optimization).
	pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

    // Fragment shader
	wgpu::FragmentState fragmentState;
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = m_shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

    // Configure blend state
	wgpu::BlendState blendState;
	// Usual alpha blending for the color:
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	// We leave the target alpha untouched:
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;

    wgpu::ColorTargetState colorTarget;
	colorTarget.format = WebGPU::GetSwapChainFormat();
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

	// We have only one target because our render pass has only one output color
	// attachment.
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;
	
	// Depth and stencil tests are not used here
	pipelineDesc.depthStencil = nullptr;

    // Multi-sampling
	// Samples per pixel
	pipelineDesc.multisample.count = 1;
	// Default value for the mask, meaning "all bits on"
	pipelineDesc.multisample.mask = ~0u;
	// Default value as well (irrelevant for count = 1 anyways)
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	// Pipeline layout
    if (m_pipelineLayout)
	    pipelineDesc.layout = m_pipelineLayout;
    else
        pipelineDesc.layout = nullptr;

    m_pipeline = WebGPU::GetDevice().createRenderPipeline(pipelineDesc);
    std::cout << "Render pipeline: " << m_pipeline << std::endl;
}

void WebGPURenderer2D::CreateVertexBuffer(const void* bufferData, uint32_t bufferLength, wgpu::VertexBufferLayout bufferLayout)
{
    std::cout << "Creating vertex buffer..." << std::endl;
    m_vertexCount = bufferLength / bufferLayout.arrayStride;
    m_vertexBufferSize = bufferLength;
    m_vertexBufferLayout = bufferLayout;
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = m_vertexBufferSize;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
    bufferDesc.mappedAtCreation = false;
    bufferDesc.label = "Vertex Buffer";
    m_vertexBuffer = WebGPU::GetDevice().createBuffer(bufferDesc);

    // Upload vertex data to the buffer
    WebGPU::GetQueue().writeBuffer(m_vertexBuffer, 0, bufferData, bufferDesc.size);
    std::cout << "Vertex buffer: " << m_vertexBuffer << std::endl;
}

void WebGPURenderer2D::CreateIndexBuffer(const std::vector<uint16_t> &bufferData)
{
    std::cout << "Creating index buffer..." << std::endl;

    m_indexCount = bufferData.size();

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = bufferData.size() * sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
    bufferDesc.label = "Index Buffer";
    m_indexBuffer = WebGPU::GetDevice().createBuffer(bufferDesc);

    // Upload index data to the buffer
    WebGPU::GetQueue().writeBuffer(m_indexBuffer, 0, bufferData.data(), bufferDesc.size);
    std::cout << "Index buffer: " << m_indexBuffer << std::endl;
}

void WebGPURenderer2D::SetBindGroupLayoutEntry(wgpu::BindGroupLayoutEntry bindGroupLayoutEntry)
{
    // Create a bind group layout
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
	bindGroupLayoutDesc.entryCount = 1;
	bindGroupLayoutDesc.entries = &bindGroupLayoutEntry;
	m_bindGroupLayout = WebGPU::GetDevice().createBindGroupLayout(bindGroupLayoutDesc);
}

void WebGPURenderer2D::CreateBindGroup()
{
    if (m_bindGroupLayout)
    {
        // Create the pipeline layout
        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;
        m_pipelineLayout = WebGPU::GetDevice().createPipelineLayout(pipelineLayoutDesc);



        // Create a binding
        wgpu::BindGroupEntry binding;
        // The index of the binding (the entries in bindGroupDesc can be in any order)
        binding.binding = 0;
        // The buffer it is actually bound to
        binding.buffer = m_uniformBuffer;
        // We can specify an offset within the buffer, so that a single buffer can hold
        // multiple uniform blocks.
        binding.offset = 0;
        // And we specify again the size of the buffer.
        assert(m_sizeOfUniform > 0);
        binding.size = m_sizeOfUniform;


        // A bind group contains one or multiple bindings
        wgpu::BindGroupDescriptor bindGroupDesc;
        bindGroupDesc.layout = m_bindGroupLayout;
        // There must be as many bindings as declared in the layout!
        bindGroupDesc.entryCount = 1; // TODO: Nisal - use bindGroupLayoutDesc.entryCount
        bindGroupDesc.entries = &binding;
        m_bindGroup = WebGPU::GetDevice().createBindGroup(bindGroupDesc);
    }
}

uint32_t WebGPURenderer2D::GetOffset(const uint32_t& uniformIndex, const uint32_t& sizeOfUniform)
{
    if (uniformIndex == 0)
        return 0;

    // Get device limits
    wgpu::SupportedLimits deviceSupportedLimits;
    WebGPU::GetDevice().getLimits(&deviceSupportedLimits);
    wgpu::Limits deviceLimits = deviceSupportedLimits.limits;
    
    /** Round 'value' up to the next multiplier of 'step' */
    auto ceilToNextMultiple = [](uint32_t value, uint32_t step) -> uint32_t
    {
        uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
        return step * divide_and_ceil;
    };

    // Create uniform buffer
    // Subtility
    assert(sizeOfUniform > 0);
    uint32_t uniformStride = ceilToNextMultiple(
        (uint32_t)sizeOfUniform,
        (uint32_t)deviceLimits.minUniformBufferOffsetAlignment
    );

    return uniformStride * uniformIndex;
}

void WebGPURenderer2D::CreateUniformBuffer(size_t bufferLength, uint32_t sizeOfUniform)
{
    assert(sizeOfUniform > 0);
    // Create uniform buffer
    // The buffer will only contain 1 float with the value of uTime
    wgpu::BufferDescriptor bufferDesc;
    const size_t maxUniformIndex = bufferLength - 1;
    bufferDesc.size = sizeOfUniform + GetOffset(maxUniformIndex, sizeOfUniform);
    // Make sure to flag the buffer as BufferUsage::Uniform
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    bufferDesc.mappedAtCreation = false;
    m_uniformBuffer = WebGPU::GetDevice().createBuffer(bufferDesc);
    m_sizeOfUniform = sizeOfUniform;
}

void WebGPURenderer2D::SetUniformData(const void* bufferData, uint32_t uniformIndex)
{
    if (m_uniformBuffer)
    {
        // WebGPU::GetQueue().writeBuffer(m_uniformBuffer, offsetof(MyUniforms, time), &bufferData.time, sizeof(MyUniforms::time));
        // WebGPU::GetQueue().writeBuffer(m_uniformBuffer, offsetof(MyUniforms, color), &bufferData.color, sizeof(MyUniforms::color));

        auto offset = GetOffset(uniformIndex, m_sizeOfUniform);
        assert(m_sizeOfUniform > 0);
        WebGPU::GetQueue().writeBuffer(m_uniformBuffer, offset, bufferData, m_sizeOfUniform);
    }
    else
    {
        assert(false);
    }
}

void WebGPURenderer2D::SimpleRender()
{
    m_renderPass.draw(m_vertexCount, 1, 0, 0);
}

void WebGPURenderer2D::Render()
{
    // Set vertex buffer while encoding the render pass
    m_renderPass.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexBufferSize);

    // Set binding group
    if (m_bindGroup)
        m_renderPass.setBindGroup(0, m_bindGroup, 0, nullptr);

    if (m_indexCount > 0)
    {
        m_renderPass.setIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint16, 0, m_indexCount * sizeof(uint16_t));
        m_renderPass.drawIndexed(m_indexCount, 1, 0, 0, 0);
    }
    else
        m_renderPass.draw(m_vertexCount, 1, 0, 0);
}

void WebGPURenderer2D::RenderIndexed(uint32_t uniformIndex, uint32_t dynamicOffsetCount)
{
    // Set vertex buffer while encoding the render pass
    m_renderPass.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexBufferSize);
    // Set index buffer while encoding the render pass
    m_renderPass.setIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint16, 0, m_indexCount * sizeof(uint16_t));

    // Set binding group
    uint32_t dynamicOffset = 0;
    if (uniformIndex > 0)
        dynamicOffset = uniformIndex * GetOffset(1, m_sizeOfUniform);
    
    m_renderPass.setBindGroup(0, m_bindGroup, dynamicOffsetCount, &dynamicOffset);
    m_renderPass.drawIndexed(m_indexCount, 1, 0, 0, 0);
}

ImTextureID WebGPURenderer2D::GetDescriptorSet()
{
    return m_textureToRenderInto;
}

void WebGPURenderer2D::BeginRenderPass()
{
    if (!m_textureToRenderInto)
        std::cerr << "Cannot acquire texture to render into" << std::endl;

    wgpu::CommandEncoderDescriptor commandEncoderDesc;
    commandEncoderDesc.label = "Renderer Command Encoder";
    m_currentCommandEncoder = WebGPU::GetDevice().createCommandEncoder(commandEncoderDesc);

    wgpu::RenderPassDescriptor renderPassDesc;

    wgpu::RenderPassColorAttachment renderPassColorAttachment;
    renderPassColorAttachment.view = m_textureToRenderInto;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
    renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
    renderPassColorAttachment.clearValue = wgpu::Color{ 0.9, 0.1, 0.2, 1.0 };
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;

    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;

    m_renderPass = m_currentCommandEncoder.beginRenderPass(renderPassDesc);

    // In its overall outline, drawing a triangle is as simple as this:
    // Select which render pipeline to use
    m_renderPass.setPipeline(m_pipeline);
}

void WebGPURenderer2D::EndRenderPass()
{
    m_renderPass.end();
    SubmitCommandBuffer();
}

void WebGPURenderer2D::SubmitCommandBuffer()
{
    wgpu::CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = "Command buffer";
    wgpu::CommandBuffer commands = m_currentCommandEncoder.finish(cmdBufferDescriptor);
    GraphicsAPI::WebGPU::GetQueue().submit(commands);
}

} // namespace GraphicsAPI