#include "OpenGLRenderingBackend.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "OpenGLGraphics.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <Walnut/Image.h>

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace Walnut
{
#ifdef __EMSCRIPTEN__
	// The page's canvas fills the browser window, so the GLFW window follows the browser window's size.
	// ImGui's own resize callback reads the canvas CSS size instead, which Emscripten's GLFW pins to the
	// GLFW window size when GLFW_SCALE_TO_MONITOR is set, so the window would never change size.
	static EM_BOOL ResizeToBrowserWindow(int /*eventType*/, const EmscriptenUiEvent* event, void* window)
	{
		glfwSetWindowSize(static_cast<GLFWwindow*>(window), event->windowInnerWidth, event->windowInnerHeight);
		return EM_FALSE;
	}
#endif

	void OpenGLRenderingBackend::Init(GLFWwindow *windowHandle, bool /*enableValidation*/)
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
		ImGui_ImplGlfw_InstallEmscriptenCallbacks(m_windowHandle, "#canvas");
		// Registered after ImGui's resize callback, so it runs last and its size wins.
		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, m_windowHandle, false, ResizeToBrowserWindow);
		glfwSetWindowSize(m_windowHandle, EM_ASM_INT({ return window.innerWidth; }), EM_ASM_INT({ return window.innerHeight; }));
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

	void OpenGLRenderingBackend::GetFramebufferSize(int& width, int& height)
	{
		glfwGetFramebufferSize(m_windowHandle, &width, &height);
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