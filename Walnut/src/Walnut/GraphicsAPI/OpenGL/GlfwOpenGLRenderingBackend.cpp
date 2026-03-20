#include "OpenGLRenderingBackend.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "OpenGLGraphics.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <glad/glad.h>
#include <Walnut/Image.h>

#include <iostream>

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

	void OpenGLRenderingBackend::CreateImGuiPipeline()
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
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void OpenGLRenderingBackend::DestroyImGuiPipeline()
    {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
    }

    void OpenGLRenderingBackend::FrameBegin()
    {
    }

    void OpenGLRenderingBackend::FrameEnd()
    {
    }

    void OpenGLRenderingBackend::FramePresent()
	{
		glfwSwapBuffers(m_windowHandle);
	}

	WalnutWindowHandleType* OpenGLRenderingBackend::GetWindowHandle()
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