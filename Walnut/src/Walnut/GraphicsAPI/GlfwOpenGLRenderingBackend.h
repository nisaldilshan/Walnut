#pragma once

#include "../RenderingBackend.h"

namespace Walnut {

	class GlfwOpenGLRenderingBackend : public RenderingBackend
	{
	public:
		virtual void Init(GLFWwindow* windowHandle) override
        {
            
        }

		void SetupGraphicsAPI() override
		{

		}

		void SetupWindow(int width, int height) override
		{

		}

		bool NeedToResizeWindow() override
		{
			return false;
		}

		void ResizeWindow(int width, int height) override
		{
			
		}

		void ConfigureImGui() override
		{

		}

		void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback) override
		{

		}

		void UploadFonts() override
		{

		}

		GLFWwindow* GetWindowHandle() override
		{
			return nullptr;
		}

		virtual void Clear() override
        {
            
        }
	};


}