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
    assert(m_shaderModule);
    pipelineDesc.vertex.module = m_shaderModule;
	pipelineDesc.vertex.entryPoint = wgpu::StringView{"vs_main"};
    pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

    // Primitive assembly and rasterization
	// Each sequence of 3 vertices is considered as a triangle
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

    // Fragment shader
	wgpu::FragmentState fragmentState;
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = m_shaderModule;
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
	// wgpu::DepthStencilState depthStencilState = wgpu::Default;
	// depthStencilState.depthCompare = wgpu::CompareFunction::Less;
	// depthStencilState.depthWriteEnabled = wgpu::OptionalBool::False;
	// depthStencilState.format = WebGPU::GetDepthFormat();
	// depthStencilState.stencilReadMask = 0;
	// depthStencilState.stencilWriteMask = 0;
    pipelineDesc.depthStencil = nullptr;
	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

    assert(m_pipelineLayout);
    pipelineDesc.layout = m_pipelineLayout;
    m_pipeline = GraphicsAPI::WebGPU::GetDevice().createRenderPipeline(pipelineDesc);
}

void WebGPUImageRenderPipeline::PrepareShaders()
{
    std::string_view shader = R"(
        struct VertexOutput {
            @builtin(position) position : vec4f,
            @location(0) fragTexCoord : vec2f
        }

        @vertex
        fn vs_main(@builtin(vertex_index) vertexIndex : u32) -> VertexOutput {
            var output : VertexOutput;
            
            // Generate UV coordinates using your exact bitwise logic:
            // Vertex 0: (0.0, 0.0)
            // Vertex 1: (2.0, 0.0)
            // Vertex 2: (0.0, 2.0)
            let uv = vec2<f32>(
                f32((vertexIndex << 1u) & 2u),
                f32(vertexIndex & 2u)
            );
            output.fragTexCoord = uv;
            
            // Map UVs to NDC positions. 
            // Note: WebGPU NDC is X:[-1, 1], Y:[-1, 1] (bottom to top), Z:[0, 1].
            output.position = vec4<f32>(uv * 2.0 - 1.0, 0.0, 1.0);
            
            return output;
        }

        @group(0) @binding(0) var tex : texture_2d<f32>;
        @group(0) @binding(1) var smp : sampler;

        @fragment
        fn fs_main(in : VertexOutput) -> @location(0) vec4f {
            // textureSample natively combines the distinct texture and sampler
            return textureSample(tex, smp, in.fragTexCoord);
        }
    )";

    wgpu::ShaderSourceWGSL shaderCodeDesc;
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
    shaderCodeDesc.code = wgpu::StringView{shader};
    
    wgpu::ShaderModuleDescriptor shaderDesc; // Connect the chain
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    m_shaderModule = GraphicsAPI::WebGPU::GetDevice().createShaderModule(shaderDesc);
}

} // namespace GraphicsAPI
