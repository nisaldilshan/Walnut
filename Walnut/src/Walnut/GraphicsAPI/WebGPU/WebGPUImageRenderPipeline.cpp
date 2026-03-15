#include "WebGPUImageRenderPipeline.h"

namespace GraphicsAPI
{
    
WebGPUImageRenderPipeline::WebGPUImageRenderPipeline(std::vector<wgpu::BindGroupLayout> &descriptorSetLayouts)
{
    PrepareShaders();
    CreatePipelineLayout(descriptorSetLayouts);
    CreatePipeline();
}

WebGPUImageRenderPipeline::~WebGPUImageRenderPipeline()
{
}

void WebGPUImageRenderPipeline::CreatePipelineLayout(const std::vector<wgpu::BindGroupLayout> &descriptorSetLayouts)
{
    // Create the pipeline layout
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)descriptorSetLayouts.data();
    m_pipelineLayout = WebGPU::GetDevice().createPipelineLayout(pipelineLayoutDesc);
    assert(m_pipelineLayout);
}

void WebGPUImageRenderPipeline::CreatePipeline()
{
    wgpu::RenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.bufferCount = 0;
    pipelineDesc.vertex.buffers = nullptr;

    // Vertex shader
    pipelineDesc.vertex.module = m_vertexShader;
	pipelineDesc.vertex.entryPoint = wgpu::StringView{"vs_main"};
    pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

    // Primitive assembly and rasterization
	// Each sequence of 3 vertices is considered as a triangle
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CW;
	pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

    // Fragment shader
	wgpu::FragmentState fragmentState;
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = m_fragmentShader;
	fragmentState.entryPoint = wgpu::StringView{"fs_main"};
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

    // Configure blend state
	wgpu::BlendState blendState;
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;

    wgpu::ColorTargetState colorTarget;
	colorTarget.format = GraphicsAPI::WebGPU::GetSwapChainFormat();
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

	// We have only one target because our render pass has only one output color
	// attachment.
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;
	
	// We setup a depth buffer state for the render pipeline
	wgpu::DepthStencilState depthStencilState = wgpu::Default;
	depthStencilState.depthCompare = wgpu::CompareFunction::Less;
	depthStencilState.depthWriteEnabled = wgpu::OptionalBool::False;
	depthStencilState.format = WebGPU::GetDepthFormat();
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;
    pipelineDesc.depthStencil = &depthStencilState;
	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	// Pipeline layout
    if (m_pipelineLayout)
	    pipelineDesc.layout = m_pipelineLayout;
    else
        pipelineDesc.layout = nullptr;

    m_pipeline = GraphicsAPI::WebGPU::GetDevice().createRenderPipeline(pipelineDesc);
}

void WebGPUImageRenderPipeline::PrepareShaders()
{
}

} // namespace GraphicsAPI
