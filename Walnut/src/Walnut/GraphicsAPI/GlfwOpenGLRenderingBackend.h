#pragma once

#include <iostream>
#include <glad/glad.h>

#include "../RenderingBackend.h"
#include "OpenGLGraphics.h"

namespace Walnut {

class GlfwOpenGLRenderingBackend : public RenderingBackend
{
public:
	void Init(GLFWwindow* windowHandle) override
	{
		m_windowHandle = windowHandle;
		glfwMakeContextCurrent(windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		gladLoadGL();
		if (!status)
		{
			std::cout << "Failed to initialize Glad!" << std::endl;
			return;
		}

		std::cout << "Graphics Info:" << std::endl;
		std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
		std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
			
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}

	void SetupGraphicsAPI() override
	{

	}

	void SetupWindow(int width, int height) override
	{

	}

	bool NeedToResizeWindow() override
	{
		return false;
	}

	void ResizeWindow(int width, int height) override
	{
		
	}

	void ConfigureImGui() override
	{

	}

	void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback) override
	{

	}

	void UploadFonts() override
	{

	}

	void FrameRender(ImDrawData* draw_data) override
	{

	}

	void FramePresent() override
	{

	}

	GLFWwindow* GetWindowHandle() override
	{
		return m_windowHandle;
	}

	void Shutdown() override
	{
		
	}

	void Cleanup() override
	{

	}

	void SetClearColor(ImVec4 color) override
	{

	}

private:
	uint32_t m_extensions_count = 0;
	const char** m_extensions = nullptr;
	GLFWwindow* m_windowHandle = nullptr;
};


}