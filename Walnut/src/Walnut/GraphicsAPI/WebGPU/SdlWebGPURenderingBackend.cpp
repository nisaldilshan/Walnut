#include "WebGPURenderingBackend.h"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "WebGPUGraphics.h"

#include "WebGPUImageRenderPipeline.h"
#include <Walnut/Image.h>
#include "WebGPUImage.h"

#include <iostream>

namespace Walnut
{
    struct WebGPUFrameBeginEndData
    {
        wgpu::CommandEncoder encoder;
        wgpu::RenderPassEncoder renderPass;
        wgpu::TextureView nextTexture;
    };
    WebGPUFrameBeginEndData g_frameData{};

    GlfwWebGPURenderingBackend::GlfwWebGPURenderingBackend()
	{}

	GlfwWebGPURenderingBackend::~GlfwWebGPURenderingBackend()
	{}

    void GlfwWebGPURenderingBackend::Init(WalnutWindowHandleType* windowHandle)
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

    void GlfwWebGPURenderingBackend::SetupWindow(int width, int height)
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
    bool GlfwWebGPURenderingBackend::NeedToResizeWindow()
    {
        return false;
    }

    void GlfwWebGPURenderingBackend::ResizeWindow(int width, int height)
    {
    }

    void GlfwWebGPURenderingBackend::CreateImGuiPipeline()
    {
        ImGui_ImplSDL3_InitForOther(m_windowHandle);

        ImGui_ImplWGPU_InitInfo initInfo{};
        initInfo.Device = GraphicsAPI::WebGPU::GetDevice();
        initInfo.NumFramesInFlight = 3;
        initInfo.RenderTargetFormat = GraphicsAPI::WebGPU::GetSwapChainFormat();
        ImGui_ImplWGPU_Init(&initInfo);
    }

    void GlfwWebGPURenderingBackend::StartImGuiFrame()
    {
        ImGui_ImplWGPU_NewFrame();
		ImGui_ImplSDL3_NewFrame();
    }

    void GlfwWebGPURenderingBackend::DestroyImGuiPipeline()
    {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        if (GraphicsAPI::WebGPU::GetSurface())
        {
            GraphicsAPI::WebGPU::GetSurface().release();
        }
    }

    void GlfwWebGPURenderingBackend::CreateMainImagePipeline(std::unique_ptr<Image> &mainImage)
    {
        auto& platformImage = mainImage->PlatformImageRef();
        std::vector<wgpu::BindGroupLayout> layouts{platformImage->GetBindGroupLayout()};                                       
        m_imageRenderPipeline = std::make_unique<GraphicsAPI::WebGPUImageRenderPipeline>(layouts);
    }

    void GlfwWebGPURenderingBackend::DestroyMainImagePipeline()
    {
    }

    void GlfwWebGPURenderingBackend::FrameBegin()
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

    void GlfwWebGPURenderingBackend::FrameEnd()
    {
        g_frameData.renderPass.end();
        g_frameData.nextTexture.release();

        wgpu::CommandBufferDescriptor cmdBufferDescriptor;
        cmdBufferDescriptor.label = wgpu::StringView("MainImage CommandBuffer");
        wgpu::CommandBuffer commands = g_frameData.encoder.finish(cmdBufferDescriptor);
        GraphicsAPI::WebGPU::GetQueue().submit(commands);
    }

    void GlfwWebGPURenderingBackend::FrameRender(std::unique_ptr<Image>& mainImage)
    {
        g_frameData.renderPass.setPipeline(m_imageRenderPipeline->GetPipeline());
        g_frameData.renderPass.setBindGroup(0, mainImage->PlatformImageRef()->GetBindGroup(), 0, nullptr);
        g_frameData.renderPass.draw(3, 1, 0, 0);
    }

    void GlfwWebGPURenderingBackend::FrameRenderImGui(void* draw_data)
    {
        ImGui_ImplWGPU_RenderDrawData((ImDrawData*)draw_data, g_frameData.renderPass);
    }

    void GlfwWebGPURenderingBackend::FramePresent()
    {
        GraphicsAPI::WebGPU::GetSurface().present();

#ifdef WEBGPU_BACKEND_WGPU
#else
        // Check for pending error callbacks
        GraphicsAPI::WebGPU::GetDevice().tick();
#endif
    }

    WalnutWindowHandleType* GlfwWebGPURenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

	void GlfwWebGPURenderingBackend::Shutdown()
	{
		
	}

	void GlfwWebGPURenderingBackend::Cleanup()
	{
	}

}