#pragma once

#include "../../RenderingBackend.h"

namespace Walnut {

class GlfwWebGPURenderingBackend : public RenderingBackend
{
public:
	void Init(GLFWwindow* windowHandle) override;

	void SetupGraphicsAPI() override;

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
	
	GLFWwindow* m_windowHandle = nullptr;
};


}