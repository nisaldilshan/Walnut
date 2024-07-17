#include "GlfwOpenGLRenderingBackend.h"

#include <iostream>

#include <imgui_impl_opengl3.h>


//#include <imgui_impl_glfw.h>
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
			return;
		}
		SDL_GL_MakeCurrent(m_windowHandle, g_SDLcontext);

		//glfwMakeContextCurrent(m_windowHandle);
		//GraphicsAPI::OpenGL::SetupOpenGL(glfwGetProcAddress);

		

		GraphicsAPI::OpenGL::SetupOpenGL(SDL_GL_GetProcAddress);
		
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
		//ImGui_ImplGlfw_InitForOpenGL(m_windowHandle, true);
		ImGui_ImplSDL2_InitForOpenGL(m_windowHandle, g_SDLcontext);
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
		//glfwSwapBuffers(m_windowHandle);
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