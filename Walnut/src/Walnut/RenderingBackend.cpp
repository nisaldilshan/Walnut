#include "GraphicsAPI/Vulkan/VulkanRenderingBackend.h"
#include "GraphicsAPI/OpenGL/OpenGLRenderingBackend.h"
#include "GraphicsAPI/WebGPU/WebGPURenderingBackend.h"

#include <cassert>

namespace Walnut {

	#if (RENDERER_BACKEND == 1)
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::OpenGL;
		typedef OpenGLRenderingBackend BackendType;
	#elif (RENDERER_BACKEND == 2)
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::Vulkan;
		typedef VulkanRenderingBackend BackendType;
	#elif (RENDERER_BACKEND == 3)
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::WebGPU;
		typedef GlfwWebGPURenderingBackend BackendType;
	#else
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::None;
	#endif

	std::unique_ptr<RenderingBackend> RenderingBackend::Create()
    {
        return std::make_unique<BackendType>();
	}

    bool RenderingBackend::SetImageToRender(uint64_t image)
    {
		if (m_renderTarget != 0) {
			return false;
		}
		m_renderTarget = image;
        return false;
    }

    bool RenderingBackend::HasImageToRender()
    {
        return m_renderTarget != 0;
    }

    uint64_t* RenderingBackend::GetImageToRender()
    {
        return &m_renderTarget;
    }
}