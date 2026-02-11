#include "OpenGLRenderingBackend.h"

#include <SDL3/SDL.h>

#include "OpenGLGraphics.h"

#define IMGUI_IMPL_OPENGL_ES3
#include <imgui_impl_opengl3.h>

#include <imgui_impl_sdl3.h>

#include <glad/glad.h>
#include <iostream>

namespace Walnut
{

	SDL_GLContext g_SDLcontext = NULL;

	void OpenGLRenderingBackend::Init(WalnutWindowHandleType* windowHandle)
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
		ImGui_ImplSDL3_InitForOpenGL(m_windowHandle, g_SDLcontext);
#if defined(__ANDROID__)
		auto result = ImGui_ImplOpenGL3_Init("#version 300 es");
		assert(result);
#else
		auto result = ImGui_ImplOpenGL3_Init("#version 410");
		assert(result);
#endif
	}

	void OpenGLRenderingBackend::StartImGuiFrame()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	void OpenGLRenderingBackend::UploadFonts()
	{
	}

	void OpenGLRenderingBackend::FrameRender(void* draw_data)
	{
		glDisable(GL_FRAMEBUFFER_SRGB); // <--- DISABLE THIS for ImGui
		ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
	}

	void OpenGLRenderingBackend::FramePresent()
	{
		SDL_GL_SwapWindow(m_windowHandle);
	}

	WalnutWindowHandleType* OpenGLRenderingBackend::GetWindowHandle()
	{
		return m_windowHandle;
	}

	void OpenGLRenderingBackend::Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
	}

	void OpenGLRenderingBackend::Cleanup()
	{
		SDL_GL_DestroyContext(g_SDLcontext);
	}

}