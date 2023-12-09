#include "WebGPURenderer3D.h"

namespace GraphicsAPI
{

void WebGPURenderer3D::CreateTextureToRenderInto(uint32_t width, uint32_t height)
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
    m_nextTexture = texture.createView(tex_view_desc);
}

void WebGPURenderer3D::CreateShaders(const char* shaderSource)
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

void WebGPURenderer3D::CreateStandaloneShader(const char *shaderSource, uint32_t vertexShaderCallCount)
{
    CreateShaders(shaderSource);
    m_vertexCount = vertexShaderCallCount;
}

void WebGPURenderer3D::CreatePipeline()
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
	
	// We setup a depth buffer state for the render pipeline
	wgpu::DepthStencilState depthStencilState = wgpu::Default;
	// Keep a fragment only if its depth is lower than the previously blended one
	depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    // Each time a fragment is blended into the target, we update the value of the Z-buffer
	depthStencilState.depthWriteEnabled = true;
	// Store the format in a variable as later parts of the code depend on it
	m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	depthStencilState.format = m_depthTextureFormat;
	// Deactivate the stencil alltogether
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

    pipelineDesc.depthStencil = &depthStencilState;

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

void WebGPURenderer3D::CreateVertexBuffer(const void* bufferData, uint32_t bufferLength, wgpu::VertexBufferLayout bufferLayout)
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

void WebGPURenderer3D::CreateIndexBuffer(const std::vector<uint16_t> &bufferData)
{
    std::cout << "Creating vertex buffer..." << std::endl;

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

void WebGPURenderer3D::SetBindGroupLayoutEntry(wgpu::BindGroupLayoutEntry bindGroupLayoutEntry)
{
    // Create a bind group layout
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
	bindGroupLayoutDesc.entryCount = 1;
	bindGroupLayoutDesc.entries = &bindGroupLayoutEntry;
	m_bindGroupLayout = WebGPU::GetDevice().createBindGroupLayout(bindGroupLayoutDesc);
}

void WebGPURenderer3D::SetSizeOfUniform(uint32_t sizeOfUniform)
{
    assert(sizeOfUniform > 0);
    m_sizeOfUniform = sizeOfUniform;

    // Get device limits
    wgpu::SupportedLimits deviceSupportedLimits;
    WebGPU::GetDevice().getLimits(&deviceSupportedLimits);
    m_deviceLimits = deviceSupportedLimits.limits;
}

void WebGPURenderer3D::CreateBindGroup()
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

uint32_t WebGPURenderer3D::GetOffset(uint32_t uniformIndex)
{
    if (uniformIndex == 0)
        return 0;
    
    /** Round 'value' up to the next multiplier of 'step' */
    auto ceilToNextMultiple = [](uint32_t value, uint32_t step) -> uint32_t
    {
        uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
        return step * divide_and_ceil;
    };

    // Create uniform buffer
    // Subtility
    assert(m_sizeOfUniform > 0);
    uint32_t uniformStride = ceilToNextMultiple(
        (uint32_t)m_sizeOfUniform,
        (uint32_t)m_deviceLimits.minUniformBufferOffsetAlignment
    );

    return uniformStride * uniformIndex;
}

void WebGPURenderer3D::CreateUniformBuffer(size_t dynamicOffsetCount)
{
    assert(m_sizeOfUniform > 0);
    m_dynamicOffsetCount = dynamicOffsetCount;
    // Create uniform buffer
    // The buffer will only contain 1 float with the value of uTime
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = m_sizeOfUniform + GetOffset(m_dynamicOffsetCount);
    // Make sure to flag the buffer as BufferUsage::Uniform
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    bufferDesc.mappedAtCreation = false;
    m_uniformBuffer = WebGPU::GetDevice().createBuffer(bufferDesc);
}

void WebGPURenderer3D::SetUniformData(const void* bufferData, uint32_t uniformIndex)
{
    if (m_uniformBuffer)
    {
        // WebGPU::GetQueue().writeBuffer(m_uniformBuffer, offsetof(MyUniforms, time), &bufferData.time, sizeof(MyUniforms::time));
        // WebGPU::GetQueue().writeBuffer(m_uniformBuffer, offsetof(MyUniforms, color), &bufferData.color, sizeof(MyUniforms::color));

        auto offset = GetOffset(uniformIndex);
        assert(m_sizeOfUniform > 0);
        WebGPU::GetQueue().writeBuffer(m_uniformBuffer, offset, bufferData, m_sizeOfUniform);
    }
    else
    {
        assert(false);
    }
}

void WebGPURenderer3D::SimpleRender()
{
    m_renderPass.draw(m_vertexCount, 1, 0, 0);
}

void WebGPURenderer3D::Render(uint32_t uniformIndex)
{
    // Set vertex buffer while encoding the render pass
    m_renderPass.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexBufferSize);

    // Set binding group
    if (m_bindGroup)
    {
        uint32_t dynamicOffset = uniformIndex * GetOffset(1);
        m_renderPass.setBindGroup(0, m_bindGroup, m_dynamicOffsetCount, &dynamicOffset);
    }

    if (m_indexCount > 0)
    {
        m_renderPass.setIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint16, 0, m_indexCount * sizeof(uint16_t));
        m_renderPass.drawIndexed(m_indexCount, 1, 0, 0, 0);
    }
    else
        m_renderPass.draw(m_vertexCount, 1, 0, 0);
}

void WebGPURenderer3D::RenderIndexed(uint32_t uniformIndex)
{
    assert(m_indexBuffer);
    // Set vertex buffer while encoding the render pass
    m_renderPass.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexBufferSize);
    // Set index buffer while encoding the render pass
    m_renderPass.setIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint16, 0, m_indexCount * sizeof(uint16_t));

    // Set binding group
    uint32_t dynamicOffset = uniformIndex * GetOffset(1);
    m_renderPass.setBindGroup(0, m_bindGroup, m_dynamicOffsetCount, &dynamicOffset);
    m_renderPass.drawIndexed(m_indexCount, 1, 0, 0, 0);
}

ImTextureID WebGPURenderer3D::GetDescriptorSet()
{
    return m_nextTexture;
}

void WebGPURenderer3D::BeginRenderPass()
{
    if (!m_nextTexture)
        std::cerr << "Cannot acquire texture to render into" << std::endl;

    wgpu::CommandEncoderDescriptor commandEncoderDesc;
    commandEncoderDesc.label = "Renderer Command Encoder";
    m_currentCommandEncoder = WebGPU::GetDevice().createCommandEncoder(commandEncoderDesc);

    wgpu::RenderPassDescriptor renderPassDesc;

    wgpu::RenderPassColorAttachment renderPassColorAttachment;
    renderPassColorAttachment.view = m_nextTexture;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
    renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
    renderPassColorAttachment.clearValue = wgpu::Color{ 0.9, 0.1, 0.2, 1.0 };
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;

    // We now add a depth/stencil attachment:
    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
    // The view of the depth texture
    depthStencilAttachment.view = m_depthTextureView;
    // The initial value of the depth buffer, meaning "far"
    depthStencilAttachment.depthClearValue = 1.0f;
    // Operation settings comparable to the color attachment
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
    // we could turn off writing to the depth buffer globally here
    depthStencilAttachment.depthReadOnly = false;

    // Stencil setup, mandatory but unused
    depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
#else
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
#endif
    depthStencilAttachment.stencilReadOnly = true;

    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;

    m_renderPass = m_currentCommandEncoder.beginRenderPass(renderPassDesc);

    // In its overall outline, drawing a triangle is as simple as this:
    // Select which render pipeline to use
    m_renderPass.setPipeline(m_pipeline);
}

void WebGPURenderer3D::EndRenderPass()
{
    m_renderPass.end();
    SubmitCommandBuffer();
}

void WebGPURenderer3D::Reset()
{
    m_vertexBuffer.destroy();
	m_vertexBuffer.release();
	m_indexBuffer.destroy();
	m_indexBuffer.release();

    // Destroy the depth texture and its view
	m_depthTextureView.release();
	m_depthTexture.destroy();
	m_depthTexture.release();
}

void WebGPURenderer3D::SubmitCommandBuffer()
{
    wgpu::CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = "Command buffer";
    wgpu::CommandBuffer commands = m_currentCommandEncoder.finish(cmdBufferDescriptor);
    GraphicsAPI::WebGPU::GetQueue().submit(commands);
}

void WebGPURenderer3D::CreateDepthTexture()
{
    // Create the depth texture
	wgpu::TextureDescriptor depthTextureDesc;
	depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
	depthTextureDesc.format = m_depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = {m_width, m_height, 1};
	depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;
	m_depthTexture = WebGPU::GetDevice().createTexture(depthTextureDesc);
	std::cout << "Depth texture: " << m_depthTexture << std::endl;

	// Create the view of the depth texture manipulated by the rasterizer
	wgpu::TextureViewDescriptor depthTextureViewDesc;
	depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
	depthTextureViewDesc.format = m_depthTextureFormat;
	m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);
	std::cout << "Depth texture view: " << m_depthTextureView << std::endl;
}

} // namespace GraphicsAPI