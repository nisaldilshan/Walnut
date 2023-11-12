#include "WebGPUGraphics.h"

#include <glm/glm.hpp>
#include "../../../../../vendor/glfw3webgpu/glfw3webgpu.h"

namespace GraphicsAPI
{

    wgpu::Device g_device = nullptr;
    wgpu::Surface g_surface = nullptr;
	wgpu::TextureFormat g_swapChainFormat = wgpu::TextureFormat::Undefined;
	wgpu::Queue g_queue = nullptr;

    void WebGPU::CreateSurface(wgpu::Instance instance, GLFWwindow* window)
    {
        std::cout << "Requesting surface..." << std::endl;
        g_surface = glfwGetWGPUSurface(instance, window);
        std::cout << "Got surface: " << g_surface << std::endl;

#ifdef WEBGPU_BACKEND_WGPU
		g_swapChainFormat = m_surface.getPreferredFormat(adapter);
#else
		g_swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif
		std::cout << "SwapChain Format: " << g_swapChainFormat << std::endl;
    }

    void WebGPU::CreateDevice(wgpu::Instance instance)
    {
        std::cout << "Requesting adapter..." << std::endl;
        wgpu::RequestAdapterOptions adapterOpts{};
		adapterOpts.compatibleSurface = g_surface;
		wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
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
		requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;
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
		g_device = adapter.requestDevice(deviceDesc);
		adapter.release();
		std::cout << "Got device: " << g_device << std::endl;

		std::cout << "Requesting queue..." << std::endl;
		g_queue = g_device.getQueue();
		std::cout << "Got queue: " << g_queue << std::endl;
    }

    void WebGPU::FreeGraphicsResources()
    {
    }

    wgpu::Surface WebGPU::GetSurface()
    {
        assert(g_surface);
        return g_surface;
    }

    wgpu::TextureFormat WebGPU::GetSwapChainFormat()
    {
		assert(g_swapChainFormat);
        return g_swapChainFormat;
    }

    wgpu::Device WebGPU::GetDevice()
    {
        assert(g_device);
        return g_device;
    }

    wgpu::Queue WebGPU::GetQueue()
    {
		assert(g_queue);
        return g_queue;
    }
}