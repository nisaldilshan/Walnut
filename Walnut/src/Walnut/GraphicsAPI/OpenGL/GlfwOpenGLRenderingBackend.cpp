#include "OpenGLRenderingBackend.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "OpenGLGraphics.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <glad/glad.h>
#include <iostream>

#include <Walnut/Image.h>

namespace Walnut
{
	OpenGLRenderingBackend::OpenGLRenderingBackend()
	{}

	OpenGLRenderingBackend::~OpenGLRenderingBackend()
	{}

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

	void OpenGLRenderingBackend::CreateMainImagePipeline(std::unique_ptr<Image>& mainImage)
    {
        auto& platformImage = mainImage->PlatformImageRef();
        // std::vector<VkDescriptorSetLayout> layouts{platformImage->GetDescriptorSetLayout()};
        // GraphicsAPI::VertexInputLayout vertexInputLayout; // vertexInputLayout disabled                                       
        // m_imageRenderPipeline = std::make_unique<GraphicsAPI::ImageRenderPipeline>(
        //     GraphicsAPI::Vulkan::GetWindowData().RenderPass, layouts, vertexInputLayout);
    }

    void OpenGLRenderingBackend::DestroyMainImagePipeline()
    {
    }

    void OpenGLRenderingBackend::FrameBegin()
    {
    }

    void OpenGLRenderingBackend::FrameRender(std::unique_ptr<Image> &mainImage)
    {
		glDisable(GL_FRAMEBUFFER_SRGB); // <--- DISABLE THIS for ImGui
		//ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
	}

	void OpenGLRenderingBackend::FrameRenderImGui(void* draw_data)
	{
		glDisable(GL_FRAMEBUFFER_SRGB); // <--- DISABLE THIS for ImGui
		ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
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