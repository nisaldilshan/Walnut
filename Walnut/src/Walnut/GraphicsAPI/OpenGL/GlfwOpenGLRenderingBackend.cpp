#define WINDOW_HANDLE_IMPL
#include "OpenGLRenderingBackend.h"
#include <iostream>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "OpenGLGraphics.h"

namespace Walnut
{

	void OpenGLRenderingBackend::Init(GLFWwindow *windowHandle)
	{
		m_windowHandle = windowHandle;
		
		glfwMakeContextCurrent(m_windowHandle);
		GraphicsAPI::OpenGL::SetupOpenGL((void*)glfwGetProcAddress);
		
	}

	void OpenGLRenderingBackend::SetupWindow(int width, int height)
	{
		GraphicsAPI::OpenGL::SetupViewport(width, height);
	}

	bool OpenGLRenderingBackend::NeedToResizeWindow()
	{
		return false;
	}

	void OpenGLRenderingBackend::ResizeWindow(int width, int height)
	{
	}

	void OpenGLRenderingBackend::ConfigureImGui()
	{
		ImGui_ImplGlfw_InitForOpenGL(m_windowHandle, true);
#ifdef __EMSCRIPTEN__
		ImGui_ImplOpenGL3_Init("#version 300 es");
#else
		ImGui_ImplOpenGL3_Init("#version 410");
#endif
	}

	void OpenGLRenderingBackend::StartImGuiFrame()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void OpenGLRenderingBackend::UploadFonts()
	{
	}

	void OpenGLRenderingBackend::FrameRender(void* draw_data)
	{
		ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
	}

	void OpenGLRenderingBackend::FramePresent()
	{
		glfwSwapBuffers(m_windowHandle);
	}

	WindowHandleType* OpenGLRenderingBackend::GetWindowHandle()
	{
		return m_windowHandle;
	}

	void OpenGLRenderingBackend::Shutdown()
	{
	}

	void OpenGLRenderingBackend::Cleanup()
	{
	}

}