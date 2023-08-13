#pragma once

#include <memory>
#include <functional>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Walnut {

	class RenderingBackend
	{
	public:
		enum class BACKEND
		{
			None = 0, Vulkan = 1, OpenGL = 2
		};
	public:
		virtual ~RenderingBackend() = default;

		virtual void Init(GLFWwindow* windowHandle) = 0;
		virtual void Clear() = 0;

        virtual void SetupGraphicsAPI() = 0;
		virtual GLFWwindow* GetWindowHandle() = 0;
		virtual void SetupWindow(int width, int height) = 0;
		virtual bool NeedToResizeWindow() = 0;
		virtual void ResizeWindow(int width, int height) = 0;
		virtual void ConfigureImGui() = 0;
		virtual void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback) = 0;
		virtual void UploadFonts() = 0;

		static BACKEND GetBackend() { return s_backend; }
		static std::unique_ptr<RenderingBackend> Create();
	private:
		static BACKEND s_backend;
	};

}
