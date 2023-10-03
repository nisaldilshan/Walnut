#include "RenderingBackend.h"
#include "GraphicsAPI/Vulkan/GlfwVulkanRenderingBackend.h"
#include "GraphicsAPI/OpenGL/GlfwOpenGLRenderingBackend.h"

#include <cassert>

namespace Walnut {

#ifdef USE_OPENGL_RENDERER
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::OpenGL;
#else
		RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::Vulkan;
#endif

    std::unique_ptr<RenderingBackend> RenderingBackend::Create()
    {
		if (s_backend == RenderingBackend::BACKEND::Vulkan)
		{
#ifndef USE_OPENGL_RENDERER
            return std::make_unique<GlfwVulkanRenderingBackend>();
#else
			assert(false); 
			return nullptr;
#endif
		}
		else if (s_backend == RenderingBackend::BACKEND::OpenGL)
		{
#ifdef USE_OPENGL_RENDERER
			return std::make_unique<GlfwOpenGLRenderingBackend>();
#else
			assert(false); 
			return nullptr;
#endif
			
		}
		else
		{
			assert(false); 
			return nullptr;
		}

		
	}

}