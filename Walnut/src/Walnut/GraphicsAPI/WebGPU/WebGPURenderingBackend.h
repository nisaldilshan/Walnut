#pragma once

#include "../../RenderingBackend.h"

namespace GraphicsAPI
{
	class WebGPUImageRenderPipeline;
}

namespace Walnut {

class WebGPURenderingBackend : public RenderingBackend
{
public:
	WebGPURenderingBackend();
	~WebGPURenderingBackend();
	WebGPURenderingBackend(const WebGPURenderingBackend&) = delete;
	WebGPURenderingBackend& operator=(const WebGPURenderingBackend&) = delete;
	WebGPURenderingBackend(WebGPURenderingBackend&&) = delete;
	WebGPURenderingBackend& operator=(WebGPURenderingBackend&&) = delete;

	void Init(WalnutWindowHandleType* windowHandle, bool enableValidation) override;
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
	bool ReadFramePixels(std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height) override;
	void FramePresent() override;
	WalnutWindowHandleType* GetWindowHandle() override;
	void Shutdown() override;
	void Cleanup() override;

private:
	WalnutWindowHandleType* m_windowHandle = nullptr;
	std::unique_ptr<GraphicsAPI::WebGPUImageRenderPipeline> m_imageRenderPipeline;
};


}