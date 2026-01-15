#pragma once

#include "../../RenderingBackend.h"

namespace Walnut {

class VulkanRenderingBackend : public RenderingBackend
{
public:
	void Init(WindowHandleType* windowHandle) override;
	void SetupWindow(int width, int height) override;
	bool NeedToResizeWindow() override;
	void ResizeWindow(int width, int height) override;
	void ConfigureImGui() override;
	void StartImGuiFrame() override;
	void UploadFonts() override;
	void FrameRender(void* draw_data) override;
	void FramePresent() override;
	WindowHandleType* GetWindowHandle() override;
	void Shutdown() override;
	void Cleanup() override;

private:
	uint32_t m_extensions_count = 0;
	const char* const* m_extensions = nullptr;
	WindowHandleType* m_windowHandle = nullptr;
};


}