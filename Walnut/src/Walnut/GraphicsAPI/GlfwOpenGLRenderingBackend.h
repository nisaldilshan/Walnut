#pragma once

#include <iostream>
#include <glad/glad.h>

#include "../RenderingBackend.h"
#include "OpenGLGraphics.h"

namespace Walnut {

static void GLFWErrorCallback(int error, const char* description)
{
	std::cout << error << " - " << description << std::endl;
	//HZ_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
}

class GlfwOpenGLRenderingBackend : public RenderingBackend
{
public:
	void Init(GLFWwindow* windowHandle) override
	{
		m_windowHandle = windowHandle;
		glfwSetErrorCallback(GLFWErrorCallback);
		glfwMakeContextCurrent(windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		gladLoadGL();
		if (!status)
			std::cout << "Failed to initialize Glad!" << std::endl;

		// HZ_CORE_INFO("OpenGL Info:");
		// HZ_CORE_INFO("    Vendor: {0}", glGetString(GL_VENDOR));
		// HZ_CORE_INFO("    Renderer: {0}", glGetString(GL_RENDERER));
		// HZ_CORE_INFO("    Version: {0}", glGetString(GL_VERSION));
		// HZ_CORE_INFO("    Extensions: {0}", glGetString(GL_EXTENSIONS));
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