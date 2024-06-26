#include "GlfwWebGPURenderingBackend.h"

#include <iostream>
#include <imgui_impl_wgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "WebGPUGraphics.h"

wgpu::SwapChain m_swapChain = nullptr;

namespace Walnut
{
    void GlfwWebGPURenderingBackend::Init(GLFWwindow *windowHandle)
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
        if (device == nullptr)
            std::cerr << "Could not initialize WebGPU Device!" << std::endl;

		

		// Add an error callback for more debug info
		std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;
		m_errorCallbackHandle = device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
			std::cout << "Device error: type " << type;
			if (message) std::cout << " (message: " << message << ")";
			std::cout << std::endl;
		});

    }

    void GlfwWebGPURenderingBackend::SetupWindow(int width, int height)
    {
        std::cout << "Creating swapchain..." << std::endl;
		wgpu::SwapChainDescriptor swapChainDesc;
		swapChainDesc.width = static_cast<uint32_t>(width);
		swapChainDesc.height = static_cast<uint32_t>(height);
		swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
		swapChainDesc.format = GraphicsAPI::WebGPU::GetSwapChainFormat();
		swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
		m_swapChain = GraphicsAPI::WebGPU::GetDevice().createSwapChain(GraphicsAPI::WebGPU::GetSurface(), swapChainDesc);
		std::cout << "Swapchain: " << m_swapChain << std::endl;
		if (m_swapChain == nullptr)
            std::cerr << "Could not initialize WebGPU SwapChain!" << std::endl;
    }
    bool GlfwWebGPURenderingBackend::NeedToResizeWindow()
    {
        return false;
    }
    void GlfwWebGPURenderingBackend::ResizeWindow(int width, int height)
    {
    }
    void GlfwWebGPURenderingBackend::ConfigureImGui()
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
		ImGui::NewFrame();
    }

    void GlfwWebGPURenderingBackend::UploadFonts()
    {
    }

    void GlfwWebGPURenderingBackend::FrameRender(ImDrawData *draw_data)
    {
        wgpu::TextureView nextTexture = m_swapChain.getCurrentTextureView();
        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            return;
        }

        wgpu::CommandEncoderDescriptor commandEncoderDesc;
        commandEncoderDesc.label = "Command Encoder";
        wgpu::CommandEncoder encoder = GraphicsAPI::WebGPU::GetDevice().createCommandEncoder(commandEncoderDesc);
        

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};
        renderPassColorAttachment.view = nextTexture;
        renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
        renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue = wgpu::Color{ 0.05, 0.05, 0.05, 1.0 };

        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.timestampWrites = nullptr;
        renderPassDesc.label = "GlfwWebGPURenderingBackend Render Pass";
        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        ImGui_ImplWGPU_RenderDrawData(draw_data, renderPass);

        renderPass.end();

        nextTexture.release();

        wgpu::CommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = "Command buffer";
        wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
        GraphicsAPI::WebGPU::GetQueue().submit(command);
    }

    void GlfwWebGPURenderingBackend::FramePresent()
    {
        m_swapChain.present();

#ifdef WEBGPU_BACKEND_WGPU
#else
        // Check for pending error callbacks
        GraphicsAPI::WebGPU::GetDevice().tick();
#endif
    }

    GLFWwindow *GlfwWebGPURenderingBackend::GetWindowHandle()
    {
        return m_windowHandle;
    }

    void GlfwWebGPURenderingBackend::Shutdown()
    {
    }
    void GlfwWebGPURenderingBackend::Cleanup()
    {
    }
    void GlfwWebGPURenderingBackend::SetClearColor(ImVec4 color)
    {
    }
}