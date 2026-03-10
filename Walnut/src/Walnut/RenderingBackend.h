#pragma once

#include <memory>
#include <functional>

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

		virtual void Init(WalnutWindowHandleType* windowHandle) = 0;
		virtual void Shutdown() = 0;
		virtual void Cleanup() = 0;

		virtual WalnutWindowHandleType* GetWindowHandle() = 0;
		virtual void SetupWindow(int width, int height) = 0;
		virtual bool NeedToResizeWindow() = 0;
		virtual void ResizeWindow(int width, int height) = 0;
		virtual void CreateImGuiPipeline() = 0;
		virtual void DestroyImGuiPipeline() = 0;
		virtual void CreateMainImagePipeline(std::unique_ptr<Image>& mainImage) = 0;
		virtual void StartImGuiFrame() = 0;
		virtual void FrameBegin() = 0;
		virtual void FrameRender(std::unique_ptr<Image>& mainImage) = 0;
		virtual void FrameRenderImGui(void* draw_data) = 0;
		virtual void FrameEnd() = 0;
		virtual void FramePresent() = 0;

		static BACKEND GetBackend() { return s_backend; }
		static std::unique_ptr<RenderingBackend> Create();

	private:
		static BACKEND s_backend;
	};

}
