#include "WebGPURenderingBackend.h"

#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "WebGPUGraphics.h"

#include "WebGPUImageRenderPipeline.h"
#include <Walnut/Image.h>
#include "WebGPUImage.h"

#include <iostream>

namespace Walnut
{
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
        ImGui_ImplGlfw_InitForOther(m_windowHandle, true);

        ImGui_ImplWGPU_InitInfo initInfo{};
        initInfo.Device = GraphicsAPI::WebGPU::GetDevice();
        initInfo.NumFramesInFlight = 3;
        initInfo.RenderTargetFormat = GraphicsAPI::WebGPU::GetSwapChainFormat();
        ImGui_ImplWGPU_Init(&initInfo);
    }

    void GlfwWebGPURenderingBackend::StartImGuiFrame()
    {
        ImGui_ImplWGPU_NewFrame();
		ImGui_ImplGlfw_NewFrame();
    }

    void GlfwWebGPURenderingBackend::DestroyImGuiPipeline()
    {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplGlfw_Shutdown();
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
    }

    void GlfwWebGPURenderingBackend::FrameEnd()
    {
    }

    void GlfwWebGPURenderingBackend::FrameRender(std::unique_ptr<Image>& mainImage)
    {
        wgpu::SurfaceTexture surfaceTexture;
        GraphicsAPI::WebGPU::GetSurface().getCurrentTexture(&surfaceTexture);
        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
            assert(false);
            return;
        }

        wgpu::Texture tex(surfaceTexture.texture); 
        wgpu::TextureView nextTexture = tex.createView();
        if (!nextTexture) {
            assert(false);
            return;
        }

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};
        renderPassColorAttachment.view = nextTexture;
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
        wgpu::CommandEncoder encoder = GraphicsAPI::WebGPU::GetDevice().createCommandEncoder(commandEncoderDesc);
        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        renderPass.setPipeline(m_imageRenderPipeline->GetPipeline());

        renderPass.setBindGroup(0, mainImage->PlatformImageRef()->GetBindGroup(), 0, nullptr);
        renderPass.draw(3, 1, 0, 0);

        renderPass.end();
        nextTexture.release();

        wgpu::CommandBufferDescriptor cmdBufferDescriptor;
        cmdBufferDescriptor.label = wgpu::StringView("MainImage CommandBuffer");
        wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
        GraphicsAPI::WebGPU::GetQueue().submit(commands);
    }

    void GlfwWebGPURenderingBackend::FrameRenderImGui(void* draw_data)
    {
        wgpu::SurfaceTexture surfaceTexture;
        GraphicsAPI::WebGPU::GetSurface().getCurrentTexture(&surfaceTexture);
        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
            // Handle resize, timeout, or lost context here
            assert(false);
            return;
        }

        wgpu::Texture tex(surfaceTexture.texture); 
        wgpu::TextureView nextTexture = tex.createView();
        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            assert(false);
            return;
        }

        wgpu::CommandEncoderDescriptor commandEncoderDesc;
        commandEncoderDesc.label = wgpu::StringView("MainImage CommandEncoder");
        wgpu::CommandEncoder encoder = GraphicsAPI::WebGPU::GetDevice().createCommandEncoder(commandEncoderDesc);
        

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};
        renderPassColorAttachment.view = nextTexture;
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
        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        ImGui_ImplWGPU_RenderDrawData((ImDrawData*)draw_data, renderPass);

        renderPass.end();

        nextTexture.release();

        wgpu::CommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = wgpu::StringView("MainImage CommandBuffer");
        wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
        GraphicsAPI::WebGPU::GetQueue().submit(command);
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