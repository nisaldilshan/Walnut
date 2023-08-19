#pragma once    

#include <memory>
#include "RenderingBackend.h"

namespace Walnut {

	class RenderCommand
	{
	public:
        inline static void Init(GLFWwindow* windowHandle)
        {
            s_RendererAPI->Init(windowHandle);
        }

		inline static void Shutdown()
        {
            s_RendererAPI->Shutdown();
        }

		inline static void Cleanup()
        {
            s_RendererAPI->Cleanup();
        }

        inline static void SetupGraphicsAPI()
        {
            s_RendererAPI->SetupGraphicsAPI();
        }

		inline static GLFWwindow* GetWindowHandle()
        {
            return s_RendererAPI->GetWindowHandle();
        }

		inline static void SetupWindow(int width, int height)
        {
            s_RendererAPI->SetupWindow(width, height);
        }

		inline static bool NeedToResizeWindow()
        {
            return s_RendererAPI->NeedToResizeWindow();
        }

		inline static void ResizeWindow(int width, int height)
        {
            s_RendererAPI->ResizeWindow(width, height);
        }

		inline static void ConfigureImGui()
        {
            s_RendererAPI->ConfigureImGui();
        }

		inline static void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback)
        {
            s_RendererAPI->StartImGuiFrame(applicationMenubarCallback, applicationUIRenderingCallback);
        }

		inline static void UploadFonts()
        {
            s_RendererAPI->UploadFonts();
        }

		inline static void FrameRender(ImDrawData* draw_data)
        {
            s_RendererAPI->FrameRender(draw_data);
        }

		inline static void FramePresent()
        {
            s_RendererAPI->FramePresent();
        }

		inline static void SetClearColor(ImVec4 color)
        {
            s_RendererAPI->SetClearColor(color);
        }

	private:
		static std::unique_ptr<RenderingBackend> s_RendererAPI;
	};

} 