#include "WebGPURenderingBackend.h"

#include "WebGPUGraphics.h"
#include "WebGPUImageRenderPipeline.h"
#include <Walnut/Image.h>
#include "WebGPUImage.h"

#include <imgui_impl_wgpu.h>

namespace Walnut 
{

struct WebGPUFrameBeginEndData
{
    wgpu::CommandEncoder encoder;
    wgpu::RenderPassEncoder renderPass;
    wgpu::TextureView nextTexture;
};
WebGPUFrameBeginEndData g_frameData{};

WebGPURenderingBackend::WebGPURenderingBackend()
{}

WebGPURenderingBackend::~WebGPURenderingBackend()
{}

void WebGPURenderingBackend::Init(WalnutWindowHandleType* windowHandle)
{
    m_windowHandle = windowHandle;

    std::vector<const char*> enabledToggles = {
        "allow_unsafe_apis",
    };
    wgpu::DawnTogglesDescriptor dawnToggles;
    dawnToggles.chain.next = nullptr;
    dawnToggles.chain.sType = wgpu::SType::DawnTogglesDescriptor;
    dawnToggles.enabledToggles = enabledToggles.data();
    dawnToggles.enabledToggleCount = enabledToggles.size();
    dawnToggles.disabledToggleCount = 0;

    wgpu::InstanceDescriptor instanceDesc{};
    instanceDesc.nextInChain = &dawnToggles.chain;

    GraphicsAPI::WebGPU::CreateInstance(instanceDesc);
    GraphicsAPI::WebGPU::CreateSurface(m_windowHandle);
    GraphicsAPI::WebGPU::CreateDevice();

    auto device = GraphicsAPI::WebGPU::GetDevice();
    if (device == nullptr) {
        std::cerr << "Could not initialize WebGPU Device!" << std::endl;
        assert(false);
        return;
    }
}

void WebGPURenderingBackend::SetupWindow(int width, int height)
{
    if (!GraphicsAPI::WebGPU::GetSurface())
    {
        assert(false);
        return;
    }
    // 1. Define the configuration (replaces SwapChainDescriptor)
    wgpu::SurfaceConfiguration config;
    config.device = GraphicsAPI::WebGPU::GetDevice();
    config.format = GraphicsAPI::WebGPU::GetSwapChainFormat();
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = static_cast<uint32_t>(width);
    config.height = static_cast<uint32_t>(height);
    config.presentMode = wgpu::PresentMode::Fifo;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    GraphicsAPI::WebGPU::GetSurface().configure(config);
}
bool WebGPURenderingBackend::NeedToResizeWindow()
{
    return false;
}

void WebGPURenderingBackend::ResizeWindow(int width, int height)
{
}

void WebGPURenderingBackend::CreateMainImagePipeline(std::unique_ptr<Image> &mainImage)
{
    auto& platformImage = mainImage->PlatformImageRef();
    std::vector<wgpu::BindGroupLayout> layouts{platformImage->GetBindGroupLayout()};                                       
    m_imageRenderPipeline = std::make_unique<GraphicsAPI::WebGPUImageRenderPipeline>(layouts);
}

void WebGPURenderingBackend::DestroyMainImagePipeline()
{
}

void WebGPURenderingBackend::FrameBegin()
{
    wgpu::SurfaceTexture surfaceTexture;
    GraphicsAPI::WebGPU::GetSurface().getCurrentTexture(&surfaceTexture);
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
        assert(false);
        return;
    }

    wgpu::Texture tex(surfaceTexture.texture); 
    g_frameData.nextTexture = tex.createView();
    if (!g_frameData.nextTexture) {
        assert(false);
        return;
    }

    wgpu::RenderPassColorAttachment renderPassColorAttachment{};
    renderPassColorAttachment.view = g_frameData.nextTexture;
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
    renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
    renderPassColorAttachment.clearValue = wgpu::Color{ 0.05, 0.05, 0.05, 1.0 };

    wgpu::RenderPassDescriptor renderPassDesc{};
    renderPassDesc.label = wgpu::StringView("MainImage RenderPass");
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.timestampWrites = nullptr;

    wgpu::CommandEncoderDescriptor commandEncoderDesc;
    commandEncoderDesc.label = wgpu::StringView("MainImage CommandEncoder");
    g_frameData.encoder = GraphicsAPI::WebGPU::GetDevice().createCommandEncoder(commandEncoderDesc);
    g_frameData.renderPass = g_frameData.encoder.beginRenderPass(renderPassDesc);
}

void WebGPURenderingBackend::FrameEnd()
{
    g_frameData.renderPass.end();
    g_frameData.nextTexture.release();

    wgpu::CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = wgpu::StringView("MainImage CommandBuffer");
    wgpu::CommandBuffer commands = g_frameData.encoder.finish(cmdBufferDescriptor);
    GraphicsAPI::WebGPU::GetQueue().submit(commands);
}

void WebGPURenderingBackend::FrameRender(std::unique_ptr<Image>& mainImage)
{
    g_frameData.renderPass.setPipeline(m_imageRenderPipeline->GetPipeline());
    g_frameData.renderPass.setBindGroup(0, mainImage->PlatformImageRef()->GetBindGroup(), 0, nullptr);
    g_frameData.renderPass.draw(3, 1, 0, 0);
}

void WebGPURenderingBackend::FrameRenderImGui(void* draw_data)
{
    ImGui_ImplWGPU_RenderDrawData((ImDrawData*)draw_data, g_frameData.renderPass);
}

void WebGPURenderingBackend::FramePresent()
{
    GraphicsAPI::WebGPU::GetSurface().present();

#ifdef WEBGPU_BACKEND_WGPU
#else
    // Check for pending error callbacks
    GraphicsAPI::WebGPU::GetDevice().tick();
#endif
}

WalnutWindowHandleType* WebGPURenderingBackend::GetWindowHandle()
{
    return m_windowHandle;
}

void WebGPURenderingBackend::Shutdown()
{
    
}

void WebGPURenderingBackend::Cleanup()
{
}

} // namespace Walnut