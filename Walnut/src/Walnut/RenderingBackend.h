#pragma once

#include <memory>
#include <functional>

#include <imgui_impl_glfw.h>

namespace Walnut {

	class RenderingBackend
	{
	public:
		enum class BACKEND
		{
			None = 0, Vulkan = 1, OpenGL = 2, WebGPU = 3
		};
		virtual ~RenderingBackend() = default;

		virtual void Init(GLFWwindow* windowHandle) = 0;
		virtual void Shutdown() = 0;
		virtual void Cleanup() = 0;

        virtual void SetupGraphicsAPI() = 0;
		virtual GLFWwindow* GetWindowHandle() = 0;
		virtual void SetupWindow(int width, int height) = 0;
		virtual bool NeedToResizeWindow() = 0;
		virtual void ResizeWindow(int width, int height) = 0;
		virtual void ConfigureImGui() = 0;
		virtual void StartImGuiFrame() = 0;
		virtual void UploadFonts() = 0;
		virtual void FrameRender(ImDrawData* draw_data) = 0;
		virtual void FramePresent() = 0;
		virtual void SetClearColor(ImVec4 color) = 0;

		static BACKEND GetBackend() { return s_backend; }
		static std::unique_ptr<RenderingBackend> Create();
	private:
		static BACKEND s_backend;
	};

}
