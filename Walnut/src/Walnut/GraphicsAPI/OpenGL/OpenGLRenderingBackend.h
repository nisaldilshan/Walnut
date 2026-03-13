#pragma once

#include "../../RenderingBackend.h"

namespace Walnut {

class OpenGLRenderingBackend : public RenderingBackend
{
public:
	OpenGLRenderingBackend();
	~OpenGLRenderingBackend();
	OpenGLRenderingBackend(const OpenGLRenderingBackend&) = delete;
	OpenGLRenderingBackend& operator=(const OpenGLRenderingBackend&) = delete;
	OpenGLRenderingBackend(OpenGLRenderingBackend&&) = delete;
	OpenGLRenderingBackend& operator=(OpenGLRenderingBackend&&) = delete;

	void Init(WalnutWindowHandleType* windowHandle) override;
	void SetupWindow(int width, int height) override;
	bool NeedToResizeWindow() override;
	void ResizeWindow(int width, int height) override;
	void CreateImGuiPipeline() override;
	void DestroyImGuiPipeline() override;
	void CreateMainImagePipeline(std::unique_ptr<Image>& mainImage) override;
	void DestroyMainImagePipeline() override;
	void StartImGuiFrame() override;
	void FrameBegin() override;
	void FrameRender(std::unique_ptr<Image>& mainImage) override;
	void FrameRenderImGui(void* draw_data) override;
	void FrameEnd() override;
	void FramePresent() override;
	WalnutWindowHandleType* GetWindowHandle() override;
	void Shutdown() override;
	void Cleanup() override;

private:
	uint32_t m_extensions_count = 0;
	const char** m_extensions = nullptr;
	WalnutWindowHandleType* m_windowHandle = nullptr;
	uint32_t m_imageRenderShaderProgram = 0;
};


}