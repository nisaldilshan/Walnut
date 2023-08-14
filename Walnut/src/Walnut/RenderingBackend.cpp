#include "RenderingBackend.h"
//#include "GraphicsAPI/GlfwVulkanRenderingBackend.h"
#include "GraphicsAPI/GlfwOpenGLRenderingBackend.h"

#include <cassert>

namespace Walnut {

	class GlfwVulkanRenderingBackend : public RenderingBackend
	{
		public:
		void Init(GLFWwindow* windowHandle) override {}
		void SetupGraphicsAPI() override {}
		void SetupWindow(int width, int height) override {}
		bool NeedToResizeWindow() override { return false; }
		void ResizeWindow(int width, int height) override {}
		void ConfigureImGui() override {}
		void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback) override {}
		void UploadFonts() override {}
		void FrameRender(ImDrawData* draw_data) override {}
		void FramePresent() override {}

		GLFWwindow* GetWindowHandle() override
		{
			return nullptr;
		}
		void Shutdown() override {}
		void Cleanup() override {}
		void SetClearColor(ImVec4 color) override {}
	};

	RenderingBackend::BACKEND RenderingBackend::s_backend = RenderingBackend::BACKEND::OpenGL;

    std::unique_ptr<RenderingBackend> RenderingBackend::Create()
    {
		switch (s_backend)
		{
			case RenderingBackend::BACKEND::None:    assert(false); return nullptr;
            case RenderingBackend::BACKEND::Vulkan:  return std::make_unique<GlfwVulkanRenderingBackend>();
			case RenderingBackend::BACKEND::OpenGL:  return std::make_unique<GlfwOpenGLRenderingBackend>();
		}

		//HZ_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}