#include "RenderingBackend.h"
#include "GraphicsAPI/Vulkan/GlfwVulkanRenderingBackend.h"
#include "GraphicsAPI/OpenGL/GlfwOpenGLRenderingBackend.h"
#include "GraphicsAPI/WebGPU/GlfwWebGPURenderingBackend.h"

#include <cassert>

namespace Walnut {

	#if (RENDERER_BACKEND == 1)
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::OpenGL;
	#elif (RENDERER_BACKEND == 2)
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::Vulkan;
	#elif (RENDERER_BACKEND == 3)
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::WebGPU;
	#else
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::None;
	#endif

	std::unique_ptr<RenderingBackend> RenderingBackend::Create()
    {
		if (s_backend == RenderingBackend::BACKEND::Vulkan)
		{
            return std::make_unique<GlfwVulkanRenderingBackend>();
		}
		else if (s_backend == RenderingBackend::BACKEND::OpenGL)
		{
			return std::make_unique<GlfwOpenGLRenderingBackend>();
		}
		else if (s_backend == RenderingBackend::BACKEND::WebGPU)
		{
			return std::make_unique<GlfwWebGPURenderingBackend>();
		}
		else
		{
			assert(false); 
			return nullptr;
		}

		
	}

}