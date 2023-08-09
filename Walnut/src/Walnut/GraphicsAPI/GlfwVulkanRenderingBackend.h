#pragma once

#include <iostream>

#include "../RenderingBackend.h"

#include "VulkanGraphics.h"

namespace Walnut {

class GlfwVulkanRenderingBackend : public RenderingBackend
{
public:
	virtual void Init() override
	{
		if (!glfwVulkanSupported())
		{
			std::cerr << "GLFW: Vulkan not supported!\n";
			return;
		}

		m_extensions = glfwGetRequiredInstanceExtensions(&m_extensions_count);
	}

	void SetupGraphicsAPI() override
	{
#ifdef __EMSCRIPTEN__
		SetupWebGPU(m_extensions, m_extensions_count);
#else
		GraphicsAPI::Vulkan::SetupVulkan(m_extensions, m_extensions_count);
#endif
	}
	
	virtual void Clear() override
	{
		
	}

private:
	uint32_t m_extensions_count = 0;
	const char** m_extensions = nullptr;
};


}