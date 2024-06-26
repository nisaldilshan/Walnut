#pragma once

#include "../../RenderingBackend.h"

namespace Walnut {

class GlfwOpenGLRenderingBackend : public RenderingBackend
{
public:
	void Init(GLFWwindow* windowHandle) override;

	void SetupWindow(int width, int height) override;

	bool NeedToResizeWindow() override;

	void ResizeWindow(int width, int height) override;

	void ConfigureImGui() override;

	void StartImGuiFrame() override;

	void UploadFonts() override;

	void FrameRender(ImDrawData* draw_data) override;

	void FramePresent() override;

	GLFWwindow* GetWindowHandle() override;
	
	void Shutdown() override;

	void Cleanup() override;

	void SetClearColor(ImVec4 color) override;

private:
	uint32_t m_extensions_count = 0;
	const char** m_extensions = nullptr;
	GLFWwindow* m_windowHandle = nullptr;
};


}