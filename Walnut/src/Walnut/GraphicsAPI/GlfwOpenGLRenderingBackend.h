#pragma once

#include <iostream>
#include <glad/glad.h>

#include "../RenderingBackend.h"
#include "OpenGLGraphics.h"

namespace Walnut {

class GlfwOpenGLRenderingBackend : public RenderingBackend
{
public:
	void Init(GLFWwindow* windowHandle) override
	{
		m_windowHandle = windowHandle;
		glfwMakeContextCurrent(windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		gladLoadGL();
		if (!status)
		{
			std::cout << "Failed to initialize Glad!" << std::endl;
			return;
		}

		std::cout << "Graphics Info:" << std::endl;
		std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
		std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
			
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}

	void SetupGraphicsAPI() override
	{

	}

	void SetupWindow(int width, int height) override
	{
		int x = 0;
		int y = 0;
		glViewport(x, y, width, height);
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
		ImGui_ImplGlfw_InitForOpenGL(m_windowHandle, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void StartImGuiFrame(const std::function<void()>& applicationMenubarCallback, const std::function<void()>& applicationUIRenderingCallback) override
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
            if (applicationMenubarCallback)
                window_flags |= ImGuiWindowFlags_MenuBar;

            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
            // and handle the pass-thru hole, so we ask Begin() to not render a background.
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
            // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
            // all active windows docked into it will lose their parent and become undocked.
            // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
            // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", nullptr, window_flags);
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO &io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            if (applicationMenubarCallback)
            {
                if (ImGui::BeginMenuBar())
                {
                    applicationMenubarCallback();
                    ImGui::EndMenuBar();
                }
            }

            if (applicationUIRenderingCallback)
                applicationUIRenderingCallback();
            else
                std::cout << "Error - applicationUIRenderingCallback is not set!" << std::endl;

            ImGui::End();
        }
	}

	void UploadFonts() override
	{

	}

	void FrameRender(ImDrawData* draw_data) override
	{
		ImGui_ImplOpenGL3_RenderDrawData(draw_data);
	}

	void FramePresent() override
	{
		glfwSwapBuffers(m_windowHandle);
	}

	GLFWwindow* GetWindowHandle() override
	{
		return m_windowHandle;
	}

	void Shutdown() override
	{
		
	}

	void Cleanup() override
	{

	}

	void SetClearColor(ImVec4 color) override
	{

	}

private:
	uint32_t m_extensions_count = 0;
	const char** m_extensions = nullptr;
	GLFWwindow* m_windowHandle = nullptr;
};


}