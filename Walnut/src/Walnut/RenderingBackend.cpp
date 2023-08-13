#include "RenderingBackend.h"
#include "GraphicsAPI/GlfwVulkanRenderingBackend.h"
#include "GraphicsAPI/GlfwOpenGLRenderingBackend.h"

#include <cassert>

namespace Walnut {

	RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::OpenGL;

    std::unique_ptr<RenderingBackend> RenderingBackend::Create()
    {
		switch (s_backend)
		{
			case RenderingBackend::BACKEND::None:    assert(false); return nullptr;
            case RenderingBackend::BACKEND::Vulkan:  return std::make_unique<GlfwVulkanRenderingBackend>();
			case RenderingBackend::BACKEND::OpenGL:  return std::make_unique<GlfwOpenGLRenderingBackend>();
		}

		//HZ_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}