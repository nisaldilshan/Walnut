#pragma once

//#include <glad/glad.h>
#include <imgui_impl_opengl3_loader.h>
#include <imgui_impl_opengl3.h>

#include "../RenderingBackend.h"

namespace Walnut {

class GlfwOpenGLRenderingBackend : public RenderingBackend
{
public:
	void Init(GLFWwindow* windowHandle) override
	{
		glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
		return nullptr;
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