#include "GlfwWebGPURenderingBackend.h"

#include <iostream>
#include <imgui_impl_wgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>


#include "../../../../../vendor/glfw3webgpu/glfw3webgpu.h"

#include <glm/glm.hpp>

wgpu::Instance m_instance = nullptr;
wgpu::Surface m_surface = nullptr;
wgpu::Device m_device = nullptr;
wgpu::TextureFormat m_swapChainFormat = wgpu::TextureFormat::Undefined;
wgpu::Queue m_queue = nullptr;
wgpu::SwapChain m_swapChain = nullptr;
wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
wgpu::Texture m_depthTexture = nullptr;
wgpu::TextureView m_depthTextureView = nullptr;


namespace Walnut
{
    void GlfwWebGPURenderingBackend::Init(GLFWwindow *windowHandle)
    {
        m_windowHandle = windowHandle;

        m_instance = createInstance(wgpu::InstanceDescriptor{});
		if (!m_instance) {
			std::cerr << "Could not initialize WebGPU!" << std::endl;
		}

		std::cout << "Requesting adapter..." << std::endl;
		m_surface = glfwGetWGPUSurface(m_instance, m_windowHandle);
		wgpu::RequestAdapterOptions adapterOpts{};
		adapterOpts.compatibleSurface = m_surface;
		wgpu::Adapter adapter = m_instance.requestAdapter(adapterOpts);
		std::cout << "Got adapter: " << adapter << std::endl;

		wgpu::SupportedLimits supportedLimits;
		adapter.getLimits(&supportedLimits);

		std::cout << "Requesting device..." << std::endl;
		wgpu::RequiredLimits requiredLimits = wgpu::Default;
		requiredLimits.limits.maxVertexAttributes = 4;
		requiredLimits.limits.maxVertexBuffers = 1;

		struct VertexAttributes {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 color;
			glm::vec2 uv;
		};
		requiredLimits.limits.maxBufferSize = 150000 * sizeof(VertexAttributes);
		requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
		requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
		requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
		requiredLimits.limits.maxInterStageShaderComponents = 8;
		requiredLimits.limits.maxBindGroups = 2;
		//                                    ^ This was a 1
		requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
		requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
		// Allow textures up to 2K
		requiredLimits.limits.maxTextureDimension1D = 2048;
		requiredLimits.limits.maxTextureDimension2D = 2048;
		requiredLimits.limits.maxTextureArrayLayers = 1;
		requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
		requiredLimits.limits.maxSamplersPerShaderStage = 1;

		wgpu::DeviceDescriptor deviceDesc;
		deviceDesc.label = "My Device";
		deviceDesc.requiredFeaturesCount = 0;
		deviceDesc.requiredLimits = &requiredLimits;
		deviceDesc.defaultQueue.label = "The default queue";
		m_device = adapter.requestDevice(deviceDesc);
		adapter.release();
		std::cout << "Got device: " << m_device << std::endl;

		// Add an error callback for more debug info
		std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;
		m_errorCallbackHandle = m_device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
			std::cout << "Device error: type " << type;
			if (message) std::cout << " (message: " << message << ")";
			std::cout << std::endl;
		});

		m_queue = m_device.getQueue();

#ifdef WEBGPU_BACKEND_WGPU
		m_swapChainFormat = m_surface.getPreferredFormat(adapter);
#else
		m_swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif

		if (m_device == nullptr)
            std::cerr << "Could not initialize WebGPU Device!" << std::endl;

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
		m_swapChain = m_device.createSwapChain(m_surface, swapChainDesc);
		std::cout << "Swapchain: " << m_swapChain << std::endl;
		if (m_swapChain == nullptr)
            std::cerr << "Could not initialize WebGPU SwapChain!" << std::endl;

        // Create the depth texture
		wgpu::TextureDescriptor depthTextureDesc;
		depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
		depthTextureDesc.format = m_depthTextureFormat;
		depthTextureDesc.mipLevelCount = 1;
		depthTextureDesc.sampleCount = 1;
		depthTextureDesc.size = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
		depthTextureDesc.viewFormatCount = 1;
		depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;
		m_depthTexture = m_device.createTexture(depthTextureDesc);
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

		if (m_depthTextureView == nullptr)
            std::cerr << "Could not initialize WebGPU DepthTexture!" << std::endl;
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
        ImGui_ImplWGPU_Init(m_device, 3, m_swapChainFormat, m_depthTextureFormat);
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
        wgpu::CommandEncoder encoder = m_device.createCommandEncoder(commandEncoderDesc);
        
        wgpu::RenderPassDescriptor renderPassDesc{};

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};
        renderPassColorAttachment.view = nextTexture;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
        renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue = wgpu::Color{ 0.05, 0.05, 0.05, 1.0 };
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
        depthStencilAttachment.view = m_depthTextureView;
        depthStencilAttachment.depthClearValue = 1.0f;
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.depthReadOnly = false;
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
        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        ImGui_ImplWGPU_RenderDrawData(draw_data, renderPass);

        renderPass.end();

        nextTexture.release();

        wgpu::CommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = "Command buffer";
        wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
        m_queue.submit(command);
    }

    void GlfwWebGPURenderingBackend::FramePresent()
    {
        m_swapChain.present();

#ifdef WEBGPU_BACKEND_WGPU
#else
        // Check for pending error callbacks
        m_device.tick();
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