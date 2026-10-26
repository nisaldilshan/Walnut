#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <vector>

#include "ExportConfig.h"
#include "WindowHandle.h"

namespace Walnut {

	class Image;

	class Walnut_API RenderingBackend
	{
	public:
		enum class BACKEND
		{
			None = 0, Vulkan = 1, OpenGL = 2, WebGPU = 3
		};
		virtual ~RenderingBackend() = default;

		// enableValidation is a request, not a guarantee: backends that don't support it (OpenGL, WebGPU) ignore it,
		// and the Vulkan backend only honors it when VK_LAYER_KHRONOS_validation is actually installed.
		virtual void Init(WalnutWindowHandleType* windowHandle, bool enableValidation) = 0;
		virtual void Shutdown() = 0;
		virtual void Cleanup() = 0;

		virtual WalnutWindowHandleType* GetWindowHandle() = 0;
		virtual void SetupWindow(int width, int height) = 0;
		virtual bool NeedToResizeWindow() = 0;
		virtual void ResizeWindow(int width, int height) = 0;
		virtual void CreateImGuiPipeline() = 0;
		virtual void DestroyImGuiPipeline() = 0;
		virtual void CreateMainImagePipeline(std::unique_ptr<Image>& mainImage) = 0;
		virtual void DestroyMainImagePipeline() = 0;
		virtual void StartImGuiFrame() = 0;
		virtual void FrameBegin() = 0;
		virtual void FrameRender(std::unique_ptr<Image>& mainImage) = 0;
		virtual void FrameRenderImGui(void* draw_data) = 0;
		virtual void FrameEnd() = 0;
		// Reads back the frame rendered to the main window, between FrameEnd and FramePresent, as RGBA8 pixels
		// with the top row first. Returns false if the backend cannot read frames back.
		virtual bool ReadFramePixels(std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height) { return false; }
		virtual void FramePresent() = 0;

		static BACKEND GetBackend() { return s_backend; }
		static std::unique_ptr<RenderingBackend> Create();

	private:
		static BACKEND s_backend;
	};

}
