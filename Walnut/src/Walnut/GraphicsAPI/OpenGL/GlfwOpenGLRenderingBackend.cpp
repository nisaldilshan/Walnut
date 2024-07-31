#include "OpenGLRenderingBackend.h"

#include <iostream>

#include <imgui_impl_opengl3.h>

#include <imgui_impl_glfw.h>

#include "../../RenderingBackend.h"
#include "OpenGLGraphics.h"

namespace Walnut
{

	void GlfwOpenGLRenderingBackend::Init(GLFWwindow *windowHandle)
	{
		m_windowHandle = windowHandle;
		
		glfwMakeContextCurrent(m_windowHandle);
		GraphicsAPI::OpenGL::SetupOpenGL((void*)glfwGetProcAddress);
		
	}

	void GlfwOpenGLRenderingBackend::SetupWindow(int width, int height)
	{
		GraphicsAPI::OpenGL::SetupViewport(width, height);
	}

	bool GlfwOpenGLRenderingBackend::NeedToResizeWindow()
	{
		return false;
	}

	void GlfwOpenGLRenderingBackend::ResizeWindow(int width, int height)
	{
	}

	void GlfwOpenGLRenderingBackend::ConfigureImGui()
	{
		ImGui_ImplGlfw_InitForOpenGL(m_windowHandle, true);
#ifdef __EMSCRIPTEN__
		ImGui_ImplOpenGL3_Init("#version 300 es");
#else
		ImGui_ImplOpenGL3_Init("#version 410");
#endif
	}

	void GlfwOpenGLRenderingBackend::StartImGuiFrame()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void GlfwOpenGLRenderingBackend::UploadFonts()
	{
	}

	void GlfwOpenGLRenderingBackend::FrameRender(void* draw_data)
	{
		ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
	}

	void GlfwOpenGLRenderingBackend::FramePresent()
	{
		glfwSwapBuffers(m_windowHandle);
	}

	WindowHandleType* GlfwOpenGLRenderingBackend::GetWindowHandle()
	{
		return m_windowHandle;
	}

	void GlfwOpenGLRenderingBackend::Shutdown()
	{
	}

	void GlfwOpenGLRenderingBackend::Cleanup()
	{
	}

}