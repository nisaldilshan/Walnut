#include "OpenGLRenderingBackend.h"

#include <iostream>

#if defined(__ANDROID__)
#define IMGUI_IMPL_OPENGL_ES3
#endif
#include <imgui_impl_opengl3.h>

#include <imgui_impl_sdl2.h>

#include "../../RenderingBackend.h"
#include "OpenGLGraphics.h"

namespace Walnut
{

	SDL_GLContext g_SDLcontext = NULL;

	void GlfwOpenGLRenderingBackend::Init(WindowHandleType* windowHandle)
	{
		m_windowHandle = windowHandle;

		g_SDLcontext = SDL_GL_CreateContext(m_windowHandle);
		if (g_SDLcontext == NULL)
		{
			std::cout << "Failed to create SDL GL context" << std::endl;
			assert(false);
			return;
		}
		SDL_GL_MakeCurrent(m_windowHandle, g_SDLcontext);		

		GraphicsAPI::OpenGL::SetupOpenGL((void*)SDL_GL_GetProcAddress);
		
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
		ImGui_ImplSDL2_InitForOpenGL(m_windowHandle, g_SDLcontext);
#if defined(__ANDROID__)
		auto result = ImGui_ImplOpenGL3_Init("#version 300 es");
		assert(result);
#else
		ImGui_ImplOpenGL3_Init("#version 410");
#endif
	}

	void GlfwOpenGLRenderingBackend::StartImGuiFrame()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
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
		SDL_GL_SwapWindow(m_windowHandle);
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