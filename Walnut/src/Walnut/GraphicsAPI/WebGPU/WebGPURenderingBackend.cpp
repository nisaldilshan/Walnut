#include "WebGPURenderingBackend.h"

#include "WebGPUGraphics.h"
#include "WebGPUImageRenderPipeline.h"
#include <Walnut/Image.h>
#include "WebGPUImage.h"

#include <imgui_impl_wgpu.h>

#include <cstring>
#include <utility>
#include <vector>

namespace Walnut 
{

struct WebGPUFrameBeginEndData
{
    wgpu::CommandEncoder encoder;
    wgpu::RenderPassEncoder renderPass;
    wgpu::Texture texture;
    wgpu::TextureView nextTexture;
};
WebGPUFrameBeginEndData g_frameData{};

WebGPURenderingBackend::WebGPURenderingBackend()
{}

WebGPURenderingBackend::~WebGPURenderingBackend()
{}

void WebGPURenderingBackend::Init(WalnutWindowHandleType* windowHandle, bool /*enableValidation*/)
{
    m_windowHandle = windowHandle;

    wgpu::InstanceDescriptor instanceDesc{};
#ifndef __EMSCRIPTEN__ // Dawn toggles only exist in native Dawn
    std::vector<const char*> enabledToggles = {
        "allow_unsafe_apis",
    };
    wgpu::DawnTogglesDescriptor dawnToggles;
    dawnToggles.chain.next = nullptr;
    dawnToggles.chain.sType = wgpu::SType::DawnTogglesDescriptor;
    dawnToggles.enabledToggles = enabledToggles.data();
    dawnToggles.enabledToggleCount = enabledToggles.size();
    dawnToggles.disabledToggleCount = 0;
    instanceDesc.nextInChain = &dawnToggles.chain;
#endif

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
    // CopySrc lets ReadFramePixels copy frames out of the surface
    config.usage = wgpu::TextureUsage::RenderAttachment | (GraphicsAPI::WebGPU::GetSurfaceUsages() & wgpu::TextureUsage::CopySrc);
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
    g_frameData.texture = tex;
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

bool WebGPURenderingBackend::ReadFramePixels(std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height)
{
#ifdef __EMSCRIPTEN__
    return false; // waiting for the buffer to be mapped would need to hand control back to the browser
#else
    wgpu::Texture texture = g_frameData.texture;
    if (!texture)
        return false;
    const wgpu::TextureFormat format = texture.getFormat();
    const bool isBGRA = format == wgpu::TextureFormat::BGRA8Unorm || format == wgpu::TextureFormat::BGRA8UnormSrgb;
    const bool isRGBA = format == wgpu::TextureFormat::RGBA8Unorm || format == wgpu::TextureFormat::RGBA8UnormSrgb;
    if (!(texture.getUsage() & wgpu::TextureUsage::CopySrc) || !(isBGRA || isRGBA))
        return false;

    auto device = GraphicsAPI::WebGPU::GetDevice();
    width = texture.getWidth();
    height = texture.getHeight();
    const uint32_t rowSize = width * 4;
    const uint32_t bytesPerRow = (rowSize + 255) / 256 * 256; // copies need rows aligned to 256 bytes
    const uint64_t size = uint64_t(bytesPerRow) * height;

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.label = wgpu::StringView("Frame readback buffer");
    bufferDesc.size = size;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    bufferDesc.mappedAtCreation = false;
    wgpu::Buffer buffer = device.createBuffer(bufferDesc);

    wgpu::TexelCopyTextureInfo source = wgpu::Default;
    source.texture = texture;
    wgpu::TexelCopyBufferInfo destination = wgpu::Default;
    destination.buffer = buffer;
    destination.layout.bytesPerRow = bytesPerRow;
    destination.layout.rowsPerImage = height;

    wgpu::CommandEncoderDescriptor encoderDesc;
    encoderDesc.label = wgpu::StringView("Frame readback CommandEncoder");
    wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);
    wgpu::Extent3D copySize;
    copySize.width = width;
    copySize.height = height;
    copySize.depthOrArrayLayers = 1;
    encoder.copyTextureToBuffer(source, destination, copySize);
    wgpu::CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = wgpu::StringView("Frame readback CommandBuffer");
    wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
    GraphicsAPI::WebGPU::GetQueue().submit(commands);
    commands.release();
    encoder.release();

    // webgpu.hpp's mapAsync(lambda) helper does not compile, so this uses the C API
    struct MapResult { bool done = false; bool mapped = false; } result;
    WGPUBufferMapCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    callbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView, void* userdata1, void*) {
        auto* result = static_cast<MapResult*>(userdata1);
        result->done = true;
        result->mapped = status == WGPUMapAsyncStatus_Success;
    };
    callbackInfo.userdata1 = &result;
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, size, callbackInfo);
    while (!result.done)
    {
        device.tick();
        GraphicsAPI::WebGPU::GetInstance().processEvents();
    }

    const bool mapped = result.mapped;
    if (mapped)
    {
        const uint8_t* data = static_cast<const uint8_t*>(buffer.getConstMappedRange(0, size));
        pixels.resize(size_t(rowSize) * height);
        for (uint32_t y = 0; y < height; y++)
            memcpy(&pixels[size_t(y) * rowSize], data + size_t(y) * bytesPerRow, rowSize);
        buffer.unmap();
        if (isBGRA)
            for (size_t i = 0; i < pixels.size(); i += 4)
                std::swap(pixels[i], pixels[i + 2]);
    }
    buffer.release();
    return mapped;
#endif
}

void WebGPURenderingBackend::FramePresent()
{
#ifdef __EMSCRIPTEN__
    // The browser presents the canvas when control returns to it at the end of the frame.
#else
    GraphicsAPI::WebGPU::GetSurface().present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
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