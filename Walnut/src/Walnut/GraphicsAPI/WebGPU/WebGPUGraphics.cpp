#include "WebGPUGraphics.h"

#include "../../GLM/GLM.h"
#include "../../../../../vendor/glfw3webgpu/glfw3webgpu.h"

namespace GraphicsAPI
{
	wgpu::Instance g_instance = nullptr;
    wgpu::Device g_device = nullptr;
    wgpu::Surface g_surface = nullptr;
	wgpu::TextureFormat g_swapChainFormat = wgpu::TextureFormat::Undefined;
	wgpu::Queue g_queue = nullptr;

	void WebGPU::CreateInstance(wgpu::InstanceDescriptor desc)
	{
		g_instance = createInstance(desc);
		if (!g_instance) {
			std::cerr << "Could not initialize WebGPU!" << std::endl;
		}
	}

    void WebGPU::CreateSurface(GLFWwindow* window)
    {
		assert(g_instance);
        std::cout << "Requesting surface..." << std::endl;
        g_surface = glfwGetWGPUSurface(g_instance, window);
        std::cout << "Got surface: " << g_surface << std::endl;

#ifdef WEBGPU_BACKEND_WGPU
		g_swapChainFormat = m_surface.getPreferredFormat(adapter);
#else
		g_swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif
		std::cout << "SwapChain Format: " << g_swapChainFormat << std::endl;
    }

    void WebGPU::CreateDevice()
    {
		assert(g_instance);
        std::cout << "Requesting adapter..." << std::endl;
        wgpu::RequestAdapterOptions adapterOpts{};
		adapterOpts.nextInChain = nullptr;
		adapterOpts.compatibleSurface = g_surface;
		wgpu::Adapter adapter = g_instance.requestAdapter(adapterOpts);
		std::cout << "Got adapter: " << adapter << std::endl;

		wgpu::SupportedLimits supportedLimits;
		adapter.getLimits(&supportedLimits);

		std::cout << "Requesting device..." << std::endl;
		wgpu::RequiredLimits requiredLimits = wgpu::Default;
		requiredLimits.limits.maxVertexAttributes = 6;
		requiredLimits.limits.maxVertexBuffers = 1;

		struct VertexAttributes {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 color;
			glm::vec2 uv;
		}; // This structure is defined just to get an idea about max values. You may define the actual VertexAttributes struct in your application.
		// TODO : use the same VertexAttributes structure in here and in the application.
		requiredLimits.limits.maxBufferSize = 150000 * sizeof(VertexAttributes);
		requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
		requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
		requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
		requiredLimits.limits.maxInterStageShaderComponents = 17;
		requiredLimits.limits.maxBindGroups = 2;
		requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;
		requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
		requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;
		// Allow textures up to 4K
		requiredLimits.limits.maxTextureDimension1D = 4096;
		requiredLimits.limits.maxTextureDimension2D = 4096;
		requiredLimits.limits.maxTextureDimension3D = 2048;
		requiredLimits.limits.maxTextureArrayLayers = 1;
		requiredLimits.limits.maxSampledTexturesPerShaderStage = 3;
		requiredLimits.limits.maxSamplersPerShaderStage = 1;

		// For Compute 
		requiredLimits.limits.maxStorageBuffersPerShaderStage = 2;
		requiredLimits.limits.maxStorageBufferBindingSize = requiredLimits.limits.maxBufferSize;
		requiredLimits.limits.maxComputeWorkgroupSizeX = 32;
		requiredLimits.limits.maxComputeWorkgroupSizeY = 1;
		requiredLimits.limits.maxComputeWorkgroupSizeZ = 1;
		requiredLimits.limits.maxComputeInvocationsPerWorkgroup = 32;
		requiredLimits.limits.maxComputeWorkgroupsPerDimension = 2;

		const char* const enabledToggles[] = {};
		wgpu::DawnTogglesDescriptor deviceTogglesDesc;
		deviceTogglesDesc.disabledToggles = enabledToggles;
		deviceTogglesDesc.disabledTogglesCount = 0;

		wgpu::DeviceDescriptor deviceDesc;
		deviceDesc.label = "My Device";
		deviceDesc.nextInChain = reinterpret_cast<wgpu::ChainedStruct*>(&deviceTogglesDesc);
		deviceDesc.requiredFeaturesCount = 0;
		deviceDesc.requiredLimits = &requiredLimits;
		deviceDesc.defaultQueue.label = "The default queue";
		g_device = adapter.requestDevice(deviceDesc);
		adapter.release();
		assert(g_device);
		std::cout << "Got device: " << g_device << std::endl;

		if (g_device)
		{
			std::cout << "Requesting queue..." << std::endl;
			g_queue = g_device.getQueue();
			std::cout << "Got queue: " << g_queue << std::endl;
		}
    }

    void WebGPU::FreeGraphicsResources()
    {
    }

    wgpu::Instance WebGPU::GetInstance()
    {
		assert(g_instance);
        return g_instance;
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