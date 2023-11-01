#include "GlfwWebGPURenderingBackend.h"

#include <iostream>
#include <imgui_impl_wgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "WebGPUGraphics.h"

wgpu::Instance m_instance = nullptr;
wgpu::TextureFormat m_swapChainFormat = wgpu::TextureFormat::Undefined;
wgpu::SwapChain m_swapChain = nullptr;

wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Undefined; // was wgpu::TextureFormat::Depth24Plus
// wgpu::Texture m_depthTexture = nullptr;
// wgpu::TextureView m_depthTextureView = nullptr;


namespace Walnut
{
    void GlfwWebGPURenderingBackend::Init(GLFWwindow *windowHandle)
    {
        m_windowHandle = windowHandle;

        m_instance = createInstance(wgpu::InstanceDescriptor{});
		if (!m_instance) {
			std::cerr << "Could not initialize WebGPU!" << std::endl;
		}

		
		GraphicsAPI::WebGPU::CreateSurface(m_instance, m_windowHandle);
        GraphicsAPI::WebGPU::CreateDevice(m_instance);

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

#ifdef WEBGPU_BACKEND_WGPU
		m_swapChainFormat = m_surface.getPreferredFormat(adapter);
#else
		m_swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif

    }
    void GlfwWebGPURenderingBackend::SetupGraphicsAPI()
    {
    }

    void GlfwWebGPURenderingBackend::SetupWindow(int width, int height)
    {
        std::cout << "Creating swapchain..." << std::endl;
		wgpu::SwapChainDescriptor swapChainDesc;
		swapChainDesc.width = static_cast<uint32_t>(width);
		swapChainDesc.height = static_cast<uint32_t>(height);
		swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
		swapChainDesc.format = m_swapChainFormat;
		swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
		m_swapChain = GraphicsAPI::WebGPU::GetDevice().createSwapChain(GraphicsAPI::WebGPU::GetSurface(), swapChainDesc);
		std::cout << "Swapchain: " << m_swapChain << std::endl;
		if (m_swapChain == nullptr)
            std::cerr << "Could not initialize WebGPU SwapChain!" << std::endl;

        // Create the depth texture
		// wgpu::TextureDescriptor depthTextureDesc;
		// depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
		// depthTextureDesc.format = m_depthTextureFormat;
		// depthTextureDesc.mipLevelCount = 1;
		// depthTextureDesc.sampleCount = 1;
		// depthTextureDesc.size = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		// depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
		// depthTextureDesc.viewFormatCount = 1;
		// depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;
		// m_depthTexture = GraphicsAPI::WebGPU::GetDevice().createTexture(depthTextureDesc);
		// std::cout << "Depth texture: " << m_depthTexture << std::endl;

		// Create the view of the depth texture manipulated by the rasterizer
		// wgpu::TextureViewDescriptor depthTextureViewDesc;
		// depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
		// depthTextureViewDesc.baseArrayLayer = 0;
		// depthTextureViewDesc.arrayLayerCount = 1;
		// depthTextureViewDesc.baseMipLevel = 0;
		// depthTextureViewDesc.mipLevelCount = 1;
		// depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
		// depthTextureViewDesc.format = m_depthTextureFormat;
		// m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);
		// std::cout << "Depth texture view: " << m_depthTextureView << std::endl;

		// if (m_depthTextureView == nullptr)
        //     std::cerr << "Could not initialize WebGPU DepthTexture!" << std::endl;
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
        ImGui_ImplWGPU_Init(GraphicsAPI::WebGPU::GetDevice(), 3, m_swapChainFormat, m_depthTextureFormat);
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
        
        wgpu::RenderPassDescriptor renderPassDesc{};

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};
        renderPassColorAttachment.view = nextTexture;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
        renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue = wgpu::Color{ 0.05, 0.05, 0.05, 1.0 };
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;

//         wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
//         depthStencilAttachment.view = m_depthTextureView;
//         depthStencilAttachment.depthClearValue = 1.0f;
//         depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
//         depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
//         depthStencilAttachment.depthReadOnly = false;
//         depthStencilAttachment.stencilClearValue = 0;
// #ifdef WEBGPU_BACKEND_WGPU
//         depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
//         depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
// #else
//         depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
//         depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
// #endif
//         depthStencilAttachment.stencilReadOnly = true;

//         renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

        renderPassDesc.timestampWriteCount = 0;
        renderPassDesc.timestampWrites = nullptr;
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