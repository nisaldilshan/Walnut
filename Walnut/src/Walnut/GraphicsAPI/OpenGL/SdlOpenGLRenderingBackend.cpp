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

	void OpenGLRenderingBackend::Init(WindowHandleType* windowHandle)
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
		ImGui_ImplSDL2_InitForOpenGL(m_windowHandle, g_SDLcontext);
#if defined(__ANDROID__)
		auto result = ImGui_ImplOpenGL3_Init("#version 300 es");
		assert(result);
#else
		ImGui_ImplOpenGL3_Init("#version 410");
#endif
	}

	void OpenGLRenderingBackend::StartImGuiFrame()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
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
		SDL_GL_SwapWindow(m_windowHandle);
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