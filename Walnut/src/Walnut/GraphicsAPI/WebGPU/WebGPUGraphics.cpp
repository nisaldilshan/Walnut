#include "WebGPUGraphics.h"

#include "../../GLM/GLM.h"

#ifdef USE_SDL
#include <sdl3webgpu.h>
#else
#include <glfw3webgpu.h>
#endif

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

    void WebGPU::CreateSurface(WalnutWindowHandleType* window)
    {
		assert(g_instance);
        std::cout << "Requesting surface..." << std::endl;
        g_surface = glfwCreateWindowWGPUSurface(g_instance, window);
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

		wgpu::Limits limits;
		adapter.getLimits(&limits);

		std::cout << "Requesting device..." << std::endl;
		wgpu::Limits requiredLimits = wgpu::Default;
		requiredLimits.maxVertexAttributes = 6;
		requiredLimits.maxVertexBuffers = 1;

		struct VertexAttributes {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 color;
			glm::vec2 uv;
		}; // This structure is defined just to get an idea about max values. You may define the actual VertexAttributes struct in your application.
		// TODO : use the same VertexAttributes structure in here and in the application.
		requiredLimits.maxBufferSize = 150000 * sizeof(VertexAttributes);
		requiredLimits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
		requiredLimits.minStorageBufferOffsetAlignment = limits.minStorageBufferOffsetAlignment;
		requiredLimits.minUniformBufferOffsetAlignment = limits.minUniformBufferOffsetAlignment;
		//requiredLimits.maxInterStageShaderComponents = 17;
		requiredLimits.maxBindGroups = 2;
		requiredLimits.maxUniformBuffersPerShaderStage = 2;
		requiredLimits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
		requiredLimits.maxDynamicUniformBuffersPerPipelineLayout = 1;
		// Allow textures up to 4K
		requiredLimits.maxTextureDimension1D = 4096;
		requiredLimits.maxTextureDimension2D = 4096;
		requiredLimits.maxTextureDimension3D = 2048;
		requiredLimits.maxTextureArrayLayers = 1;
		requiredLimits.maxSampledTexturesPerShaderStage = 3;
		requiredLimits.maxSamplersPerShaderStage = 1;

		// For Compute 
		requiredLimits.maxStorageBuffersPerShaderStage = 2;
		requiredLimits.maxStorageBufferBindingSize = requiredLimits.maxBufferSize;
		requiredLimits.maxComputeWorkgroupSizeX = 32;
		requiredLimits.maxComputeWorkgroupSizeY = 1;
		requiredLimits.maxComputeWorkgroupSizeZ = 1;
		requiredLimits.maxComputeInvocationsPerWorkgroup = 32;
		requiredLimits.maxComputeWorkgroupsPerDimension = 2;

		auto errorCallback = [](WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
			std::cout << "Device error: type " << type;
			if (message.length > 0) std::cout << " (message: " << message.data << ")";
			std::cout << std::endl;
		};

		wgpu::UncapturedErrorCallbackInfo callbackInfo = {};
		callbackInfo.callback = errorCallback;
		callbackInfo.userdata1 = nullptr;
		callbackInfo.userdata2 = nullptr;

		wgpu::DeviceDescriptor deviceDesc;
		//deviceDesc.label = "My Device";
		deviceDesc.nextInChain = nullptr;
		deviceDesc.requiredFeatureCount = 0;
		deviceDesc.requiredLimits = &requiredLimits;
		deviceDesc.uncapturedErrorCallbackInfo = callbackInfo;
		deviceDesc.defaultQueue.nextInChain = nullptr;
		//deviceDesc.defaultQueue.label = "The default queue";
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