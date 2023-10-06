#pragma once

#include "../../RenderingBackend.h"

#include <dawn/webgpu.h>

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

	void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback) override;

	void UploadFonts() override;

	void FrameRender(ImDrawData* draw_data) override;

	void FramePresent() override;

	GLFWwindow* GetWindowHandle() override;
	
	void Shutdown() override;

	void Cleanup() override;

	void SetClearColor(ImVec4 color) override;

private:
	WGPUDevice m_wgpu_device;
	GLFWwindow* m_windowHandle = nullptr;
};


}